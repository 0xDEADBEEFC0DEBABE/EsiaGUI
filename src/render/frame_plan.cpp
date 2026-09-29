// WGT UI - frame planner implementation (see frame_plan.hpp)
#include "render/frame_plan.hpp"
#include "render/gpu_common.hpp"

namespace wgt
{
    namespace
    {
        struct Mapper
        {
            ImVec2 off, scale;
            PxRect operator()(float x0, float y0, float x1, float y1) const
            {
                return {(x0 - off.x) * scale.x, (y0 - off.y) * scale.y, (x1 - off.x) * scale.x, (y1 - off.y) * scale.y};
            }
            PxRect operator()(const ImVec4& r) const { return (*this)(r.x, r.y, r.z, r.w); }
        };

        // Must mirror the quad expansion in FxVS (wgt_fx.hlsl).
        void InstanceExtent(const fx::Instance& in, float out[4])
        {
            const std::uint32_t feat = in.flags[0];
            float x0 = in.rect[0], y0 = in.rect[1], x1 = in.rect[2], y1 = in.rect[3];
            if (feat & fx::kMerge)
            {
                x0 = std::min(x0, in.shape2[0]);
                y0 = std::min(y0, in.shape2[1]);
                x1 = std::max(x1, in.shape2[2]);
                y1 = std::max(y1, in.shape2[3]);
            }
            float pad = 2.0f;
            if (feat & fx::kStroke)
                pad = std::max(pad, in.strokeParams[0] * in.strokeParams[1] + 2.0f);
            if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow))
                pad = std::max(pad, in.shadowParams[0] * 1.6f + std::max(in.shadowParams[1], 0.0f) +
                                        std::max(std::fabs(in.shadowParams[2]), std::fabs(in.shadowParams[3])) + 2.0f);
            if (feat & fx::kGlow)
                pad = std::max(pad, in.glowParams[0] * 1.8f + 2.0f);
            x0 -= pad; y0 -= pad; x1 += pad; y1 += pad;
            if (feat & fx::kMask)
            {
                x0 = std::max(x0, in.mask[0] - 1.0f);
                y0 = std::max(y0, in.mask[1] - 1.0f);
                x1 = std::min(x1, in.mask[2] + 1.0f);
                y1 = std::min(y1, in.mask[3] + 1.0f);
            }
            out[0] = x0; out[1] = y0; out[2] = x1; out[3] = y1;
        }

        // Distance beyond the shape where its shadow / glow still reaches a visible opacity (> 4%): past it the
        // tails vanish once seen through blurred glass (capture planning ignores them).
        float InstanceCoreExtent(const fx::Instance& in)
        {
            const std::uint32_t feat = in.flags[0];
            float pad = 1.0f;
            if (feat & fx::kStroke)
                pad = std::max(pad, in.strokeParams[0] * in.strokeParams[1] + 1.0f);
            if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow) && in.shadow[3] > 0.04f)
                pad = std::max(pad, in.shadowParams[0] * 0.5f + std::max(in.shadowParams[1], 0.0f) +
                                        std::max(std::fabs(in.shadowParams[2]), std::fabs(in.shadowParams[3])));
            if (feat & fx::kGlow)
            {
                const float peak = in.glowParams[1] * in.glow[3];
                if (peak > 0.04f)
                    pad = std::max(pad, in.glowParams[0] * std::sqrt(std::log(peak / 0.04f) / 2.2f));
            }
            return pad;
        }

        PxRect VertexBounds(const ImDrawList* dl, const ImDrawCmd& cmd, const Mapper& map)
        {
            const ImDrawIdx* idx = dl->IdxBuffer.Data + cmd.IdxOffset;
            const ImDrawVert* vtx = dl->VtxBuffer.Data + cmd.VtxOffset;
            float x0 = FLT_MAX, y0 = FLT_MAX, x1 = -FLT_MAX, y1 = -FLT_MAX;
            for (unsigned int i = 0; i < cmd.ElemCount; ++i)
            {
                const ImVec2 p = vtx[idx[i]].pos;
                x0 = std::min(x0, p.x);
                y0 = std::min(y0, p.y);
                x1 = std::max(x1, p.x);
                y1 = std::max(y1, p.y);
            }
            if (x1 < x0)
                return {};
            return map(x0, y0, x1, y1).Expand(1.0f);
        }

        bool SameClip(const PxRect& a, const PxRect& b) { return a.x0 == b.x0 && a.y0 == b.y0 && a.x1 == b.x1 && a.y1 == b.y1; }
    }

    void FramePlan::Build(const ImDrawData* dd, ImDrawCallback resetCallback)
    {
        ops.clear();
        instances.clear();
        anyGlass = false;
        anyLayer = false;
        fxCount = 0;
        totalVtx = dd->TotalVtxCount;
        totalIdx = dd->TotalIdxCount;

        const Mapper map{dd->DisplayPos, dd->FramebufferScale};
        const ImDrawCallback fxCallback = fx::CommandCallback();

        // Pass 1: do we need precise bounds at all? (only glass capture / glow layers use them)
        bool needBounds = false;
        for (const ImDrawList* dl : dd->CmdLists)
        {
            for (const ImDrawCmd& cmd : dl->CmdBuffer)
            {
                if (cmd.UserCallback != fxCallback)
                    continue;
                fx::CommandHeader h;
                if (!fx::ReadHeader(cmd, h))
                    continue;
                if (h.kind == fx::CommandKind::LayerBegin)
                    needBounds = true;
                else if (h.kind == fx::CommandKind::Shape && cmd.UserCallbackDataSize >= (int)sizeof(fx::ShapeCommand))
                {
                    std::uint32_t feat = 0;
                    std::memcpy(&feat, (const char*)cmd.UserCallbackData + offsetof(fx::ShapeCommand, instance) + offsetof(fx::Instance, flags), sizeof(feat));
                    if (feat & (fx::kGlass | fx::kCustom))
                        needBounds = true;
                }
            }
            if (needBounds)
                break;
        }

        std::vector<int> layerStack;
        auto addToLayers = [&](const PxRect& b) {
            for (int li : layerStack)
                ops[li].bounds = ops[li].bounds.Union(b);
        };

        int globalVtx = 0, globalIdx = 0;
        for (const ImDrawList* dl : dd->CmdLists)
        {
            int openBatch = -1;
            float fade[4] = {0, 0, 0, 0};   // edge fade of this draw list (FadeBegin / FadeEnd)
            for (int ci = 0; ci < dl->CmdBuffer.Size; ++ci)
            {
                const ImDrawCmd& cmd = dl->CmdBuffer[ci];
                const PxRect clip = map(cmd.ClipRect);

                if (cmd.UserCallback == fxCallback)
                {
                    fx::CommandHeader h;
                    if (!fx::ReadHeader(cmd, h))
                        continue;
                    if (h.kind == fx::CommandKind::Shape)
                    {
                        fx::ShapeCommand sc;
                        if (!fx::ReadShape(cmd, sc) || clip.Empty())
                            continue;
                        const fx::Instance& in = sc.instance;
                        float e[4];
                        InstanceExtent(in, e);
                        const PxRect full = map(e[0], e[1], e[2], e[3]);
                        PxRect b = full.Intersect(clip);
                        if (b.Empty())
                            continue;
                        // the shape plus the part of its shadow / glow that is still visible (> 4%): the faint outer
                        // tails vanish through blurred glass, so they never force a new backdrop capture
                        PxRect shapeR = map(in.rect[0], in.rect[1], in.rect[2], in.rect[3]);
                        if (in.flags[0] & fx::kMerge)
                            shapeR = shapeR.Union(map(in.shape2[0], in.shape2[1], in.shape2[2], in.shape2[3]));
                        // glass and user effects (which may call WgtBackdrop()) read the captured backdrop
                        const bool glass = (in.flags[0] & (fx::kGlass | fx::kCustom)) != 0;
                        // Liquid glass shapes are tracked apart from other content: like SwiftUI's glass effect
                        // container, neighbouring glass shares one backdrop and only sees glass it sits on.
                        const bool glassShape = (in.flags[0] & fx::kGlass) && !(in.flags[0] & fx::kCustom);
                        const PxRect core = glassShape ? PxRect{} : shapeR.Expand(InstanceCoreExtent(in) * map.scale.x).Intersect(full).Intersect(clip);
                        const PxRect gShape = glassShape ? shapeR.Intersect(clip) : PxRect{};
                        PxRect gRegion, gCore;
                        // user effects may sample any blur level: -1 = build the whole pyramid. Glass reads its frost,
                        // the blurred surroundings its rim reflects and, with legibility, a wide neighbourhood.
                        // (the exposure reads exactly one level: the smallest radius whose LevelsForBlur builds it)
                        const float glassBlurPx = std::max(in.glass[0], kGlassEnvBlur) * map.scale.x;
                        const float ambientPx = in.shape[0] > 0.0f ? (float)(1 << AmbientLevel(map.scale.x)) : 0.0f;
                        const float blurPx = (in.flags[0] & fx::kCustom) ? -1.0f : std::max(glassBlurPx, ambientPx);
                        // WgtSampleBackdrop reads level 0 below a 4 px frost (the rim reflection and the exposure
                        // always read blurred levels)
                        const bool reads0 = (in.flags[0] & fx::kCustom) || ((in.flags[0] & fx::kGlass) && in.glass[0] * map.scale.x < 4.0f);
                        if (glass)
                        {
                            const bool isGlass = (in.flags[0] & fx::kGlass) != 0;
                            // frost footprint, plus the rim reflection sampled just outside the edge (GlassEnvReach)
                            const float halfMin = 0.5f * std::min(in.rect[2] - in.rect[0], in.rect[3] - in.rect[1]);
                            const float reach = in.glass[0] * 2.0f + GlassEnvReach(std::min(in.glass[2], std::max(halfMin, 1.0f))) + kGlassEnvBlur;
                            const float margin = (isGlass ? reach : 40.0f) * map.scale.x + 16.0f;
                            // refraction samples inward from the rim, so only the blur footprint reaches outside
                            const float coreMargin = (isGlass ? in.glass[0] : 40.0f) * map.scale.x + 4.0f;
                            gRegion = map(in.rect[0], in.rect[1], in.rect[2], in.rect[3]).Expand(margin);
                            gCore = map(in.rect[0], in.rect[1], in.rect[2], in.rect[3]).Expand(coreMargin);
                            anyGlass = true;
                        }

                        bool append = openBatch >= 0 && openBatch == (int)ops.size() - 1;
                        if (append)
                        {
                            const RenderOp& o = ops[openBatch];
                            append = SameClip(o.clip, clip) && o.texture == h.texture && o.effect == h.effect;
                            // a glass shape must see what the batch already drew under it -> new batch (new capture)
                            if (append && glass && o.bounds.Overlaps(gRegion.Intersect(clip)))
                                append = false;
                        }
                        if (append)
                        {
                            RenderOp& o = ops[openBatch];
                            ++o.instCount;
                            o.bounds = o.bounds.Union(b);
                            o.core = o.core.Union(core);
                            o.glassShape = o.glassShape.Union(gShape);
                            if (glass)
                            {
                                o.glassRegion = o.glass ? o.glassRegion.Union(gRegion) : gRegion;
                                o.glassCore = o.glass ? o.glassCore.Union(gCore) : gCore;
                                o.blurPx = !o.glass ? blurPx : ((o.blurPx < 0.0f || blurPx < 0.0f) ? -1.0f : std::max(o.blurPx, blurPx));
                                o.readsLevel0 = o.readsLevel0 || reads0;
                                o.glass = true;
                            }
                        }
                        else
                        {
                            RenderOp o;
                            o.type = RenderOp::FxBatch;
                            o.clip = clip;
                            o.bounds = b;
                            o.core = core;
                            o.glassShape = gShape;
                            std::memcpy(o.fade, fade, sizeof(fade));
                            o.texture = h.texture;
                            o.effect = h.effect;
                            o.instStart = (std::uint32_t)instances.size();
                            o.instCount = 1;
                            o.glass = glass;
                            o.glassRegion = gRegion;
                            o.glassCore = gCore;
                            o.blurPx = glass ? blurPx : 0.0f;
                            o.readsLevel0 = glass && reads0;
                            ops.push_back(o);
                            openBatch = (int)ops.size() - 1;
                        }
                        instances.push_back(in);
                        ++fxCount;
                        addToLayers(b);
                        continue;
                    }
                    if (h.kind == fx::CommandKind::FadeBegin || h.kind == fx::CommandKind::FadeEnd)
                    {
                        fx::FadeCommand fc;
                        if (h.kind == fx::CommandKind::FadeBegin && fx::ReadFade(cmd, fc))
                        {
                            fade[0] = (fc.params.y0 - map.off.y) * map.scale.y;
                            fade[1] = (fc.params.y1 - map.off.y) * map.scale.y;
                            fade[2] = fc.params.top * map.scale.y;
                            fade[3] = fc.params.bottom * map.scale.y;
                        }
                        else
                            fade[0] = fade[1] = fade[2] = fade[3] = 0.0f;
                        openBatch = -1;   // batches never mix fades
                        continue;
                    }
                    if (h.kind == fx::CommandKind::LayerBegin)
                    {
                        fx::LayerCommand lc;
                        if (!fx::ReadLayer(cmd, lc))
                            continue;
                        RenderOp o;
                        o.type = RenderOp::LayerBegin;
                        o.layer = lc.params;
                        ops.push_back(o);
                        layerStack.push_back((int)ops.size() - 1);
                        anyLayer = true;
                        openBatch = -1;
                        continue;
                    }
                    if (h.kind == fx::CommandKind::LayerEnd)
                    {
                        if (!layerStack.empty())
                        {
                            const int begin = layerStack.back();
                            layerStack.pop_back();
                            RenderOp o;
                            o.type = RenderOp::LayerEnd;
                            o.layer = ops[begin].layer;
                            // halo extends beyond the content
                            ops[begin].bounds = ops[begin].bounds.Expand(ops[begin].layer.radius * map.scale.x * 2.5f + 8.0f);
                            o.bounds = ops[begin].bounds;
                            o.blurPx = ops[begin].layer.radius * map.scale.x;
                            ops.push_back(o);
                            addToLayers(o.bounds);
                        }
                        openBatch = -1;
                        continue;
                    }
                    continue;
                }

                if (cmd.UserCallback != nullptr)
                {
                    RenderOp o;
                    o.type = (resetCallback && cmd.UserCallback == resetCallback) ? RenderOp::ResetState : RenderOp::UserCallback;
                    o.clip = clip;
                    o.list = dl;
                    o.cmd = &cmd;
                    ops.push_back(o);
                    openBatch = -1;
                    continue;
                }

                if (cmd.ElemCount == 0 || clip.Empty())
                    continue;
                RenderOp o;
                o.type = RenderOp::Draw;
                o.clip = clip;
                std::memcpy(o.fade, fade, sizeof(fade));
                o.texture = cmd.GetTexID();
                const ImTextureData* td = cmd.TexRef._TexData;
                o.lcd = td != nullptr && td->LcdCoverage;
                o.coverage = td != nullptr && (td->Format == ImTextureFormat_Alpha8 || o.lcd);
                o.idxCount = cmd.ElemCount;
                o.idxOffset = cmd.IdxOffset + (std::uint32_t)globalIdx;
                o.vtxOffset = cmd.VtxOffset + (std::uint32_t)globalVtx;
                o.bounds = needBounds ? VertexBounds(dl, cmd, map).Intersect(clip) : clip;
                ops.push_back(o);
                addToLayers(o.bounds);
                openBatch = -1;
            }
            globalVtx += dl->VtxBuffer.Size;
            globalIdx += dl->IdxBuffer.Size;
        }

        // close unterminated layers
        while (!layerStack.empty())
        {
            const int begin = layerStack.back();
            layerStack.pop_back();
            RenderOp o;
            o.type = RenderOp::LayerEnd;
            o.layer = ops[begin].layer;
            ops[begin].bounds = ops[begin].bounds.Expand(ops[begin].layer.radius * map.scale.x * 2.5f + 8.0f);
            o.bounds = ops[begin].bounds;
            o.blurPx = ops[begin].layer.radius * map.scale.x;
            ops.push_back(o);
        }
        if (anyGlass)
            PlanCaptures();
    }

    // ------------------------------------------------------------------ capture planning
    // Glass must see everything drawn before it, so its backdrop is captured (copy + blur pyramid) right before
    // it draws - but only when something was drawn inside the region it samples since the last capture. The
    // whole frame is known here, so a capture is also stretched over the later glass batches that nothing
    // draws under in between: one capture serves a window and every glass control inside it, instead of one
    // capture per control. Drawing is tracked as a list of rectangles (not one growing box), so unrelated
    // draws elsewhere never force a new capture.
    namespace
    {
        struct DirtyRects
        {
            void Clear() { rects.clear(); }
            std::vector<PxRect> rects;
            void Add(const PxRect& r)
            {
                if (r.Empty())
                    return;
                if (rects.size() >= 64)
                {
                    PxRect u = rects[0];
                    for (const PxRect& x : rects)
                        u = u.Union(x);
                    rects.assign(1, u);
                }
                rects.push_back(r);
            }
            bool Overlaps(const PxRect& r) const
            {
                for (const PxRect& x : rects)
                    if (x.Overlaps(r))
                        return true;
                return false;
            }
        };

        // What was drawn since a capture, split by kind: glass sees ordinary content anywhere in its blur
        // footprint, but other glass only where it actually sits on it (a glass button on a glass card).
        struct Drawn
        {
            DirtyRects content, glass;
            void Clear()
            {
                content.Clear();
                glass.Clear();
            }
            void Add(const RenderOp& op)
            {
                switch (op.type)
                {
                case RenderOp::FxBatch:
                    content.Add(op.core);
                    glass.Add(op.glassShape);
                    break;
                case RenderOp::UserCallback:
                    content.Add(op.clip);
                    break;
                default:
                    content.Add(op.bounds);
                    break;
                }
            }
            bool Hides(const RenderOp& glassOp) const
            {
                return content.Overlaps(glassOp.glassCore) || glass.Overlaps(glassOp.glassShape);
            }
        };

        float Area(const PxRect& r) { return r.Empty() ? 0.0f : (r.x1 - r.x0) * (r.y1 - r.y0); }
        int LevelsOf(const RenderOp& op) { return op.blurPx < 0.0f ? kBackdropLevels - 1 : LevelsForBlur(op.blurPx); }
    }

    void FramePlan::PlanCaptures()
    {
        plannedCaptures = 0;
        bool valid = false;
        PxRect validRegion;
        int validLevels = 0;
        bool validLevel0 = false;
        Drawn drawn;   // since the last capture
        int depth = 0;
        const int n = (int)ops.size();
        for (int i = 0; i < n; ++i)
        {
            RenderOp& op = ops[i];
            switch (op.type)
            {
            case RenderOp::LayerBegin:
                ++depth;
                break;
            case RenderOp::LayerEnd:
                if (depth > 0 && --depth == 0)
                {
                    valid = false;   // the bloom pyramid replaced the backdrop pyramid
                    drawn.Add(op);
                }
                break;
            case RenderOp::FxBatch:
                if (op.glass)
                {
                    const int levels = LevelsOf(op);
                    if (!(valid && validRegion.Contains(op.glassRegion) && validLevels >= levels && (validLevel0 || !op.readsLevel0) &&
                          !drawn.Hides(op)))
                    {
                        PxRect region = op.glassRegion;
                        int lv = levels;
                        bool l0 = op.readsLevel0;
                        float areaSum = Area(region);
                        Drawn ahead;
                        ahead.Add(op);
                        int d = depth;
                        for (int k = i + 1; k < n && k < i + 512; ++k)
                        {
                            const RenderOp& q = ops[k];
                            if (q.type == RenderOp::LayerBegin)
                            {
                                ++d;
                                continue;
                            }
                            if (q.type == RenderOp::LayerEnd)
                            {
                                if (d > 0 && --d == 0)
                                    break;   // the pyramid gets overwritten there
                                continue;
                            }
                            if (q.type == RenderOp::FxBatch && q.glass)
                            {
                                if (ahead.Hides(q))
                                    break;   // q recaptures and replaces the pyramid: later batches cannot use this capture
                                // served by this capture as long as the merged region does not become mostly empty
                                const PxRect u = region.Union(q.glassRegion);
                                const float sum = areaSum + Area(q.glassRegion);
                                if (Area(u) > sum * 1.6f + 128.0f * 128.0f)
                                    break;   // q captures its own region, same as above
                                region = u;
                                areaSum = sum;
                                lv = std::max(lv, LevelsOf(q));
                                l0 = l0 || q.readsLevel0;
                            }
                            if (d == 0 && (q.type == RenderOp::Draw || q.type == RenderOp::FxBatch || q.type == RenderOp::UserCallback))
                                ahead.Add(q);
                        }
                        op.captureRegion = region;
                        op.captureLevels = lv;
                        op.captureLevel0 = l0;
                        valid = true;
                        validRegion = region;
                        validLevels = lv;
                        validLevel0 = l0;
                        drawn.Clear();
                        ++plannedCaptures;
                    }
                }
                if (depth == 0)
                    drawn.Add(op);
                break;
            case RenderOp::Draw:
            case RenderOp::UserCallback:
                if (depth == 0)
                    drawn.Add(op);
                break;
            default:
                break;
            }
        }
    }
}
