// Esia - frame planner (see esia/render/frame_plan.hpp). The capture decisions are WGT's; batching also joins draws
// across others that touch different pixels, which WGT did not.
#include "esia/render/frame_plan.hpp"
#include "gpu_constants.hpp"
#include <cfloat>
#include <cmath>
#include <cstring>

namespace esia::render
{
    namespace
    {
        struct Mapper
        {
            Vec2 off, scale;
            PxRect operator()(float x0, float y0, float x1, float y1) const
            {
                return {(x0 - off.x) * scale.x, (y0 - off.y) * scale.y, (x1 - off.x) * scale.x, (y1 - off.y) * scale.y};
            }
            PxRect operator()(const Rect& r) const { return (*this)(r.min.x, r.min.y, r.max.x, r.max.y); }
        };

        // The data instances that carry a path's points after it (fx::ShapeKind::Path): they go wherever the path goes,
        // in the same piece of the same batch, never on their own (at most what is left of the command)
        std::uint32_t PathData(const fx::Instance& in, std::uint32_t left)
        {
            if (!(in.flags[0] & fx::kPath) || in.flags[2] != (std::uint32_t)fx::ShapeKind::Path)
                return 0;
            const std::uint32_t n = (std::uint32_t)std::max(in.radii[2], 0.0f);
            return std::min((n + fx::kPathPointsPerData - 1) / fx::kPathPointsPerData, left);
        }

        // The shape an instance draws, before its shadow / glow: its rect (with a liquid merge's second rect), or a
        // glyph's field (its rect is the text's box). Must mirror FxVS (esia_fx.hlsl).
        Rect ShapeBox(const fx::Instance& in)
        {
            const std::uint32_t feat = in.flags[0];
            if (feat & fx::kGlyph)
                return Rect(in.shape2[0], in.shape2[1], in.shape2[2], in.shape2[3]);
            Rect r(in.rect[0], in.rect[1], in.rect[2], in.rect[3]);
            if (feat & fx::kMerge)
                r = r.Union(Rect(in.shape2[0], in.shape2[1], in.shape2[2], in.shape2[3]));
            return r;
        }

        // How far a drop shadow's offset reaches: along x or y as it is, or every way when a light turns it
        float ShadowReach(const fx::Instance& in, bool lit)
        {
            return lit ? std::sqrt(in.shadowParams[2] * in.shadowParams[2] + in.shadowParams[3] * in.shadowParams[3])
                       : std::max(std::fabs(in.shadowParams[2]), std::fabs(in.shadowParams[3]));
        }

        // Must mirror the quad expansion in FxVS (esia_fx.hlsl).
        void InstanceExtent(const fx::Instance& in, float out[4], bool lit)
        {
            const std::uint32_t feat = in.flags[0];
            const Rect shape = ShapeBox(in);
            float x0 = shape.min.x, y0 = shape.min.y, x1 = shape.max.x, y1 = shape.max.y;
            if (feat & fx::kMotion)   // the area the shape sweeps
            {
                const float mx = std::fabs(in.custom[0]) * 0.5f, my = std::fabs(in.custom[1]) * 0.5f;
                x0 -= mx;
                y0 -= my;
                x1 += mx;
                y1 += my;
            }
            float pad = 2.0f;
            if (feat & fx::kStroke)
                pad = std::max(pad, in.strokeParams[0] * in.strokeParams[1] + 2.0f);
            if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow))
                pad = std::max(pad, in.shadowParams[0] * 1.6f + std::max(in.shadowParams[1], 0.0f) + ShadowReach(in, lit) + 2.0f);
            if (feat & fx::kGlow)
                pad = std::max(pad, in.glowParams[0] * 1.8f + 2.0f);
            if (feat & fx::kLegible)
                pad = std::max(pad, in.radii[3] * 1.8f + 2.0f);
            x0 -= pad;
            y0 -= pad;
            x1 += pad;
            y1 += pad;
            if (feat & fx::kMask)
            {
                x0 = std::max(x0, in.mask[0] - 1.0f);
                y0 = std::max(y0, in.mask[1] - 1.0f);
                x1 = std::min(x1, in.mask[2] + 1.0f);
                y1 = std::min(y1, in.mask[3] + 1.0f);
            }
            out[0] = x0;
            out[1] = y0;
            out[2] = x1;
            out[3] = y1;
        }

        // Distance beyond the shape where its shadow / glow still reaches a visible opacity (> 4%): past it the
        // tails vanish once seen through blurred glass (capture planning ignores them).
        float InstanceCoreExtent(const fx::Instance& in, bool lit)
        {
            const std::uint32_t feat = in.flags[0];
            float pad = 1.0f;
            if (feat & fx::kMotion)
                pad = std::max(pad, std::max(std::fabs(in.custom[0]), std::fabs(in.custom[1])) * 0.5f + 1.0f);
            if (feat & fx::kStroke)
                pad = std::max(pad, in.strokeParams[0] * in.strokeParams[1] + 1.0f);
            if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow) && in.shadow[3] > 0.04f)
                pad = std::max(pad, in.shadowParams[0] * 0.5f + std::max(in.shadowParams[1], 0.0f) + ShadowReach(in, lit));
            if (feat & fx::kGlow)
            {
                const float peak = in.glowParams[1] * in.glow[3];
                if (peak > 0.04f)
                    pad = std::max(pad, in.glowParams[0] * std::sqrt(std::log(peak / 0.04f) / 2.2f));
            }
            if ((feat & fx::kLegible) && in.radii[2] * 0.85f > 0.04f)   // the legibility halo, at its strongest
                pad = std::max(pad, in.radii[3] * std::sqrt(std::log(in.radii[2] * 0.85f / 0.04f) / 2.2f));
            return pad;
        }

        // What of an FX instance other draws may not cross while batching: the shape, and its shadow / glow as far as
        // they are visible (> 4%, InstanceCoreExtent's measure; the shadow in its own box, offset only the way it is
        // drawn). Past it a shadow's tail changes a pixel by a few levels at most, which drawing a neighbouring label
        // before or after it does not show - and a label beside a button would otherwise split every row of a form
        // into a batch of shapes and a batch of text.
        void InstanceJoinReach(const fx::Instance& in, float out[4], bool lit)
        {
            const std::uint32_t feat = in.flags[0];
            const Rect shape = ShapeBox(in);
            float x0 = shape.min.x, y0 = shape.min.y, x1 = shape.max.x, y1 = shape.max.y;
            if (feat & fx::kMotion)
            {
                const float mx = std::fabs(in.custom[0]) * 0.5f, my = std::fabs(in.custom[1]) * 0.5f;
                x0 -= mx;
                y0 -= my;
                x1 += mx;
                y1 += my;
            }
            const float sx0 = x0, sy0 = y0, sx1 = x1, sy1 = y1;
            float pad = 1.0f;
            if (feat & fx::kStroke)
                pad = std::max(pad, in.strokeParams[0] * in.strokeParams[1] + 1.0f);
            if (feat & fx::kGlow)
            {
                const float peak = in.glowParams[1] * in.glow[3];
                if (peak > 0.04f)
                    pad = std::max(pad, in.glowParams[0] * std::sqrt(std::log(peak / 0.04f) / 2.2f));
            }
            x0 -= pad;
            y0 -= pad;
            x1 += pad;
            y1 += pad;
            if ((feat & fx::kShadow) && !(feat & fx::kInnerShadow) && in.shadow[3] > 0.04f)
            {
                // the shadow in its own box, offset the way it is drawn - any way when a light turns it
                const float r = in.shadowParams[0] * 0.5f + std::max(in.shadowParams[1], 0.0f);
                const float reach = lit ? ShadowReach(in, true) : 0.0f;
                x0 = std::min(x0, sx0 + (lit ? -reach : in.shadowParams[2]) - r);
                y0 = std::min(y0, sy0 + (lit ? -reach : in.shadowParams[3]) - r);
                x1 = std::max(x1, sx1 + (lit ? reach : in.shadowParams[2]) + r);
                y1 = std::max(y1, sy1 + (lit ? reach : in.shadowParams[3]) + r);
            }
            if (feat & fx::kMask)
            {
                x0 = std::max(x0, in.mask[0] - 1.0f);
                y0 = std::max(y0, in.mask[1] - 1.0f);
                x1 = std::min(x1, in.mask[2] + 1.0f);
                y1 = std::min(y1, in.mask[3] + 1.0f);
            }
            out[0] = x0;
            out[1] = y0;
            out[2] = x1;
            out[3] = y1;
        }

        // The pixels a geometry command touches: its vertices' bounds, which the list keeps as they are written
        // (DrawCmd::vtxBounds), plus a pixel
        PxRect VertexBounds(const DrawCmd& cmd, const Mapper& map)
        {
            if (cmd.vtxEnd <= cmd.vtxFirst)
                return {};
            return map(cmd.vtxBounds).Expand(1.0f);
        }

        void CloseLayer(std::vector<RenderOp>& ops, int begin, float scale)
        {
            RenderOp o;
            o.type = RenderOp::LayerEnd;
            o.layer = ops[(std::size_t)begin].layer;
            // the halo extends beyond the content
            ops[(std::size_t)begin].bounds = ops[(std::size_t)begin].bounds.Expand(ops[(std::size_t)begin].layer.radius * scale * 2.5f + 8.0f);
            o.bounds = ops[(std::size_t)begin].bounds;
            o.blurPx = ops[(std::size_t)begin].layer.radius * scale;
            ops.push_back(o);
        }

        constexpr int kJoinLookBack = 64;                         // ops searched for a batch to join
        constexpr std::uint32_t kNoPiece = 0xFFFFFFFFu;

        float AreaOf(const PxRect& r) { return r.Empty() ? 0.0f : (r.x1 - r.x0) * (r.y1 - r.y0); }
    }

    void FramePlan::Region::Add(const PxRect& r)
    {
        if (r.Empty())
            return;
        if (count > 0)
        {
            // the next glyph run or shape along a row: one rect with the last while that adds little - a label and
            // a button title with a gap between them stay two
            PxRect& last = rects[count - 1];
            const PxRect u = last.Union(r);
            if (AreaOf(u) <= (AreaOf(last) + AreaOf(r)) * 1.25f)
            {
                last = u;
                return;
            }
        }
        if (count == kMax)
        {
            // full: neighbours in drawing order merge pairwise (they lie near each other; the newest stay apart)
            for (int i = 0; i < kMax / 2; ++i)
                rects[i] = rects[2 * i].Union(rects[2 * i + 1]);
            count = kMax / 2;
        }
        rects[count++] = r;
    }

    bool FramePlan::Region::Overlaps(const PxRect& r) const
    {
        for (int i = 0; i < count; ++i)
            if (rects[i].Overlaps(r))
                return true;
        return false;
    }

    void FramePlan::Build(const DrawData& dd, const TextureInfoFn& textureInfo, bool directGeometry)
    {
        ops.clear();
        instances.clear();
        vertices.clear();
        indices.clear();
        lists.clear();
        geometry.clear();
        instanceRuns.clear();
        quadRuns.clear();
        quadCount = 0;
        direct = directGeometry;
        totalVertices = totalIndices = 0;
        pieces_.clear();
        opFirst_.clear();
        opLast_.clear();
        layerStack_.clear();
        regions_.clear();
        anyGlass = false;
        anyLayer = false;
        fxCount = 0;
        plannedCaptures = 0;
        if (!direct)
        {
            vertices.reserve(dd.TotalVertices());
            indices.reserve(dd.TotalIndices());
            instances.reserve(dd.TotalFx());
        }

        const Mapper map{dd.displayPos, dd.framebufferScale};

        std::vector<int>& layerStack = layerStack_;   // kept from frame to frame, as every buffer here: no allocation
        auto addToLayers = [&](const PxRect& b) {
            for (int li : layerStack)
                ops[(std::size_t)li].bounds = ops[(std::size_t)li].bounds.Union(b);
        };

        // What each op draws is a chain of pieces (runs of instances or indices), laid out op by op at the end: a
        // draw that joins an earlier batch lands behind that batch's own instances or indices.
        auto link = [&](int op, const Piece& piece) {
            opFirst_.resize(ops.size(), kNoPiece);
            opLast_.resize(ops.size(), kNoPiece);
            const std::uint32_t last = opLast_[(std::size_t)op];
            if (last != kNoPiece)
            {
                Piece& l = pieces_[last];   // continues the op's last piece: consecutive instances or indices of a list
                if ((piece.inst && l.inst && l.inst + l.count == piece.inst) || (piece.quad && l.quad && l.quad + l.count == piece.quad) ||
                    (piece.idx && l.idx && l.idx + l.count == piece.idx && l.base == piece.base))
                {
                    l.count += piece.count;
                    return;
                }
            }
            const std::uint32_t id = (std::uint32_t)pieces_.size();
            pieces_.push_back(piece);
            if (last == kNoPiece)
                opFirst_[(std::size_t)op] = id;
            else
                pieces_[last].next = id;
            opLast_[(std::size_t)op] = id;
        };

        // The batch a new draw joins: the latest op it is compatible with, as long as every op after that one touches
        // other pixels - drawing the new one before them then changes nothing. Buttons and their labels alternate
        // between FX instances and text; this draws the shapes of a window in few batches and its text in few draws.
        // Glass (and user effects) keeps its place: it joins only the last op, nothing moves across it, and only the
        // last op joins it, so the backdrop captures stay where they were and plain shapes do not take on the glass
        // shader. Ops before a layer (it redirects drawing) or a callback (it draws what it likes) are never joined.
        int barrier = 0;
        // Scissors. A draw its clip does not cut (all of it lies inside) looks the same under any scissor that holds it
        // whole: such draws batch with draws of other clips. Controls push a clip of their own around their shadow
        // (ScopedUnclip) between labels clipped by the window's body - without this, each one was a draw of its own.
        // A batch of such draws only has a free clip; the first draw its clip does cut fixes the batch's scissor.
        auto clipJoins = [](const RenderOp& o, const PxRect& clip, const PxRect& whole, bool cut) {
            if (o.clipFree)
                return !cut || clip.Contains(o.bounds);
            return cut ? o.clip == clip : o.clip.Contains(whole);
        };
        auto joinClip = [](RenderOp& o, const PxRect& clip, bool cut) {
            if (o.clipFree && cut)
            {
                o.clipFree = false;
                o.clip = clip;
            }
        };
        auto findJoin = [&](const PxRect& reach, bool glass, const auto& compatible) {
            const int n = (int)ops.size();
            for (int k = n - 1; k >= barrier && k >= n - kJoinLookBack; --k)
            {
                const RenderOp& o = ops[(std::size_t)k];
                if (compatible(o) && (k == n - 1 || !o.glass))
                    return k;
                if (glass || o.glass)
                    return -1;
                // the batch's bounds first (cheap), then the rects of what it draws
                if (o.bounds.Overlaps(reach) && ((std::size_t)k >= regions_.size() || regions_[(std::size_t)k].Overlaps(reach)))
                    return -1;
            }
            return -1;
        };
        // what op k touches: its draws' rects (ops other than draws and FX batches have none and are never passed)
        auto touch = [&](int k, const PxRect& r) {
            if (regions_.size() <= (std::size_t)k)
                regions_.resize((std::size_t)k + 1);
            regions_[(std::size_t)k].Add(r);
        };

        for (const DrawList* dl : dd.lists)
        {
            const std::uint32_t vtxBase = totalVertices, idxBase = totalIndices;
            totalVertices += (std::uint32_t)dl->Vertices().size();
            totalIndices += (std::uint32_t)dl->Indices().size();
            if (direct)
                lists.push_back({dl, vtxBase, idxBase});
            else
                vertices.insert(vertices.end(), dl->Vertices().begin(), dl->Vertices().end());

            float fade[4] = {0, 0, 0, 0};   // edge fade of this draw list (FadeBegin / FadeEnd)
            for (const DrawCmd& cmd : dl->Commands())
            {
                const PxRect clip = map(cmd.clip);
                switch (cmd.kind)
                {
                case DrawCmdKind::Fx:
                {
                    if (clip.Empty())
                        break;
                    for (std::uint32_t ii = cmd.first; ii < cmd.first + cmd.count; ++ii)
                    {
                        const fx::Instance& in = dl->FxInstances()[ii];
                        const std::uint32_t data = PathData(in, cmd.first + cmd.count - ii - 1);
                        float e[4];
                        InstanceExtent(in, e, lightShadows);
                        const PxRect full = map(e[0], e[1], e[2], e[3]);
                        const PxRect b = full.Intersect(clip);
                        if (b.Empty())
                        {
                            ii += data;   // a path out of sight: its points too
                            continue;
                        }
                        // glass keeps its clip: its capture regions are planned within it
                        // the shape plus the part of its shadow / glow that is still visible (> 4%): the faint outer
                        // tails vanish through blurred glass, so they never force a new backdrop capture
                        const PxRect shapeR = map(ShapeBox(in));
                        // glass, user effects (which may call WgtBackdrop()) and legible text read the captured backdrop
                        const bool glass = (in.flags[0] & (fx::kGlass | fx::kCustom | fx::kLegible)) != 0;
                        // Liquid glass shapes are tracked apart from other content: like SwiftUI's glass effect
                        // container, neighbouring glass shares one backdrop and only sees glass it sits on.
                        const bool glassShape = (in.flags[0] & fx::kGlass) && !(in.flags[0] & fx::kCustom);
                        const PxRect core = glassShape ? PxRect{} : shapeR.Expand(InstanceCoreExtent(in, lightShadows) * map.scale.x).Intersect(full).Intersect(clip);
                        const PxRect gShape = glassShape ? shapeR.Intersect(clip) : PxRect{};
                        const bool cut = glass || !clip.Contains(full);
                        PxRect gRegion, gCore, g0;
                        // user effects may sample any blur level: -1 = build the whole pyramid. Glass reads its frost, the
                        // blurred surroundings its rim reflects and, with legibility, a wide neighbourhood (the exposure
                        // reads exactly one level: the smallest radius whose LevelsForBlur builds it)
                        float blurPx = 0.0f;
                        bool reads0 = false;
                        if (glass)
                        {
                            const float glassBlurPx = std::max(in.glass[0], kGlassEnvBlur) * map.scale.x;
                            const float ambientPx = in.shape[0] > 0.0f ? (float)(1 << AmbientLevel(map.scale.x)) : 0.0f;
                            const float legiblePx = (in.flags[0] & fx::kLegible) ? in.radii[3] * map.scale.x : 0.0f;   // as blurred as its halo is wide
                            blurPx = (in.flags[0] & fx::kCustom) ? -1.0f : std::max({glassBlurPx, ambientPx, legiblePx});
                            // WgtSampleBackdrop reads level 0 below a 4 px frost (the rim reflection and the exposure
                            // always read blurred levels)
                            reads0 = (in.flags[0] & fx::kCustom) || ((in.flags[0] & fx::kGlass) && in.glass[0] * map.scale.x < 4.0f);
                            const bool isGlass = (in.flags[0] & fx::kGlass) != 0;
                            // frost footprint, plus the rim reflection sampled just outside the edge (GlassEnvReach)
                            const float halfMin = 0.5f * std::min(in.rect[2] - in.rect[0], in.rect[3] - in.rect[1]);
                            const float reach = in.glass[0] * 2.0f + GlassEnvReach(std::min(in.glass[2], std::max(halfMin, 1.0f))) + kGlassEnvBlur;
                            const float wide = (in.flags[0] & fx::kLegible) ? std::max(40.0f, in.radii[3] * 2.0f) : 40.0f;   // a wide halo's blur
                            const float margin = (isGlass ? reach : wide) * map.scale.x + 16.0f;
                            gRegion = map(in.rect[0], in.rect[1], in.rect[2], in.rect[3]).Expand(margin);
                            // what glass shows of its backdrop is what lies under the shape (refraction pulls inwards):
                            // content drawn beside it after the capture would reach its frost only faintly, so it does
                            // not force a new capture. A user effect may read anywhere around it.
                            gCore = glassShape ? gShape.Expand(2.0f) : map(in.rect[0], in.rect[1], in.rect[2], in.rect[3]).Expand(40.0f * map.scale.x + 4.0f);
                            // level 0 is read inside the shape only (refraction and the loupe pull inwards, dispersion
                            // stays within the pull), plus a bilinear footprint; a user effect may read anywhere
                            g0 = isGlass && !(in.flags[0] & fx::kCustom) ? shapeR.Expand(2.0f).Intersect(clip) : gRegion;
                            anyGlass = true;
                        }

                        // what it may not cross (glass keeps its place: its whole reach)
                        PxRect jr = b;
                        if (!glass)
                        {
                            InstanceJoinReach(in, e, lightShadows);
                            jr = map(e[0], e[1], e[2], e[3]).Intersect(b);
                        }
                        int k = findJoin(jr, glass, [&](const RenderOp& o) {
                            return o.type == RenderOp::FxBatch && clipJoins(o, clip, full, cut) && o.texture == cmd.texture && o.effect == cmd.effect &&
                                   std::equal(fade, fade + 4, o.fade) &&
                                   // a glass shape must see what the batch already drew under it -> new batch (new capture)
                                   !(glass && o.bounds.Overlaps(gRegion.Intersect(clip)));
                        });
                        if (k >= 0)
                        {
                            RenderOp& o = ops[(std::size_t)k];
                            joinClip(o, clip, cut);
                            o.instCount += 1 + data;
                            o.features |= in.flags[0];
                            o.bounds = o.bounds.Union(b);
                            o.core = o.core.Union(core);
                            o.glassShape = o.glassShape.Union(gShape);
                            if (glass)
                            {
                                o.glassRegion = o.glass ? o.glassRegion.Union(gRegion) : gRegion;
                                o.glassCore = o.glass ? o.glassCore.Union(gCore) : gCore;
                                o.blurPx = !o.glass ? blurPx : ((o.blurPx < 0.0f || blurPx < 0.0f) ? -1.0f : std::max(o.blurPx, blurPx));
                                o.readsLevel0 = o.readsLevel0 || reads0;
                                if (reads0)
                                    o.level0Region = o.level0Region.Union(g0);
                                o.glass = true;
                            }
                        }
                        else
                        {
                            RenderOp o;
                            o.type = RenderOp::FxBatch;
                            o.clip = clip;
                            o.clipFree = !cut;
                            o.bounds = b;
                            o.core = core;
                            o.glassShape = gShape;
                            std::copy(fade, fade + 4, o.fade);
                            o.texture = cmd.texture;
                            o.effect = cmd.effect;
                            o.instCount = 1 + data;
                            o.features = in.flags[0];
                            o.glass = glass;
                            o.glassRegion = gRegion;
                            o.glassCore = gCore;
                            o.blurPx = glass ? blurPx : 0.0f;
                            o.readsLevel0 = glass && reads0;
                            o.level0Region = o.readsLevel0 ? g0 : PxRect{};
                            ops.push_back(o);
                            k = (int)ops.size() - 1;
                        }
                        link(k, Piece{&in, nullptr, 1 + data, 0, kNoPiece});
                        touch(k, jr);
                        fxCount += 1 + (int)data;
                        addToLayers(b);
                        ii += data;
                    }
                    break;
                }
                case DrawCmdKind::FadeBegin:
                {
                    const fx::FadeParams& f = dl->Fades()[cmd.payload];
                    fade[0] = (f.y0 - map.off.y) * map.scale.y;
                    fade[1] = (f.y1 - map.off.y) * map.scale.y;
                    fade[2] = f.top * map.scale.y;
                    fade[3] = f.bottom * map.scale.y;
                    break;
                }
                case DrawCmdKind::FadeEnd:
                    fade[0] = fade[1] = fade[2] = fade[3] = 0.0f;
                    break;
                case DrawCmdKind::LayerBegin:
                {
                    RenderOp o;
                    o.type = RenderOp::LayerBegin;
                    o.layer = dl->Layers()[cmd.payload];
                    ops.push_back(o);
                    layerStack.push_back((int)ops.size() - 1);
                    anyLayer = true;
                    barrier = (int)ops.size();
                    break;
                }
                case DrawCmdKind::LayerEnd:
                    if (!layerStack.empty())
                    {
                        const int begin = layerStack.back();
                        layerStack.pop_back();
                        CloseLayer(ops, begin, map.scale.x);
                        addToLayers(ops.back().bounds);
                        barrier = (int)ops.size();
                    }
                    break;
                case DrawCmdKind::Callback:
                {
                    RenderOp o;
                    o.type = RenderOp::Callback;
                    o.clip = clip;
                    o.list = dl;
                    o.cmd = &cmd;
                    ops.push_back(o);
                    barrier = (int)ops.size();
                    break;
                }
                case DrawCmdKind::Quads:
                {
                    // as geometry, drawn instanced: they join draws of quads only
                    if (cmd.count == 0 || clip.Empty())
                        break;
                    RenderOp o;
                    o.type = RenderOp::Draw;
                    o.quads = true;
                    o.clip = clip;
                    std::copy(fade, fade + 4, o.fade);
                    o.texture = cmd.texture;
                    o.anyTexture = cmd.anyTexture;
                    o.shapes = cmd.shapes;
                    TextureInfo ti;
                    if (!cmd.anyTexture && cmd.texture != 0 && textureInfo && textureInfo(cmd.texture, ti))
                    {
                        o.coverage = ti.Coverage();
                        o.lcd = ti.Lcd();
                    }
                    o.quadCount = cmd.count;
                    const PxRect whole = map(cmd.vtxBounds).Expand(1.0f);
                    const bool cut = !clip.Contains(whole);
                    o.bounds = whole.Intersect(clip);
                    if (o.bounds.Empty())
                        break;   // nothing inside the clip
                    o.clipFree = !cut;
                    // plain shapes read no texture: they join quads on any, and a batch of them alone takes any
                    int k = findJoin(o.bounds, false, [&](const RenderOp& x) {
                        return x.type == RenderOp::Draw && x.quads && clipJoins(x, clip, whole, cut) &&
                               (o.anyTexture || x.anyTexture || (x.texture == cmd.texture && x.coverage == o.coverage)) && std::equal(fade, fade + 4, x.fade);
                    });
                    if (k >= 0)
                    {
                        RenderOp& x = ops[(std::size_t)k];
                        joinClip(x, clip, cut);
                        if (x.anyTexture && !o.anyTexture)
                        {
                            x.texture = o.texture;
                            x.coverage = o.coverage;
                            x.lcd = o.lcd;
                            x.anyTexture = false;
                        }
                        x.quadCount += cmd.count;
                        x.shapes = x.shapes || cmd.shapes;
                        x.bounds = x.bounds.Union(o.bounds);
                    }
                    else
                    {
                        ops.push_back(o);
                        k = (int)ops.size() - 1;
                    }
                    Piece pc;
                    pc.quad = dl->Quads().data() + cmd.first;
                    pc.count = cmd.count;
                    pc.next = kNoPiece;
                    link(k, pc);
                    touch(k, o.bounds);
                    addToLayers(o.bounds);
                    break;
                }
                case DrawCmdKind::Geometry:
                {
                    if (cmd.count == 0 || clip.Empty())
                        break;
                    RenderOp o;
                    o.type = RenderOp::Draw;
                    o.clip = clip;
                    std::copy(fade, fade + 4, o.fade);
                    o.texture = cmd.texture;
                    TextureInfo ti;
                    if (cmd.texture != 0 && textureInfo && textureInfo(cmd.texture, ti))
                    {
                        o.coverage = ti.Coverage();
                        o.lcd = ti.Lcd();
                    }
                    o.idxCount = cmd.count;
                    const PxRect whole = VertexBounds(cmd, map);
                    const bool cut = !clip.Contains(whole);
                    o.bounds = whole.Intersect(clip);
                    if (o.bounds.Empty())
                        break;   // nothing inside the clip
                    o.clipFree = !cut;
                    int k = findJoin(o.bounds, false, [&](const RenderOp& x) {
                        return x.type == RenderOp::Draw && !x.quads && clipJoins(x, clip, whole, cut) && x.texture == cmd.texture && x.coverage == o.coverage &&
                               std::equal(fade, fade + 4, x.fade);
                    });
                    if (k >= 0)
                    {
                        joinClip(ops[(std::size_t)k], clip, cut);
                        ops[(std::size_t)k].idxCount += cmd.count;
                        ops[(std::size_t)k].bounds = ops[(std::size_t)k].bounds.Union(o.bounds);
                    }
                    else
                    {
                        ops.push_back(o);
                        k = (int)ops.size() - 1;
                    }
                    link(k, Piece{nullptr, dl->Indices().data() + cmd.first, cmd.count, vtxBase, kNoPiece, idxBase + cmd.first});
                    touch(k, o.bounds);
                    addToLayers(o.bounds);
                    break;
                }
                }
            }
        }

        // close unterminated layers
        while (!layerStack.empty())
        {
            const int begin = layerStack.back();
            layerStack.pop_back();
            CloseLayer(ops, begin, map.scale.x);
        }

        // the instances and indices, batch by batch
        const PxRect screen = map(dd.displayPos.x, dd.displayPos.y, dd.displayPos.x + dd.displaySize.x, dd.displayPos.y + dd.displaySize.y);
        opFirst_.resize(ops.size(), kNoPiece);
        std::uint32_t instanceCount = 0;
        for (std::size_t k = 0; k < ops.size(); ++k)
        {
            RenderOp& o = ops[k];
            // A free clip: any scissor that holds what the batch draws on the target draws the same - the one before
            // it when that does (no scissor change), else the whole target.
            if (o.clipFree)
                o.clip = k > 0 && ops[k - 1].clip.Contains(o.bounds.Intersect(screen)) ? ops[k - 1].clip : screen;
            if (o.type == RenderOp::Draw && o.quads)
            {
                // the quads stay in the lists, as runs (the renderer copies them to the GPU once)
                o.quadStart = quadCount;
                for (std::uint32_t p = opFirst_[k]; p != kNoPiece; p = pieces_[p].next)
                {
                    const Piece& pc = pieces_[p];
                    if (!quadRuns.empty() && quadRuns.back().first + quadRuns.back().count == pc.quad)
                        quadRuns.back().count += pc.count;
                    else
                        quadRuns.push_back({pc.quad, pc.count});
                    quadCount += pc.count;
                }
                continue;
            }
            if (direct)
            {
                // the instances as runs, the draws at their lists' offsets
                if (o.type == RenderOp::FxBatch)
                    o.instStart = instanceCount;
                else if (o.type == RenderOp::Draw)
                    o.geometryFirst = (std::uint32_t)geometry.size();
                for (std::uint32_t p = opFirst_[k]; p != kNoPiece; p = pieces_[p].next)
                {
                    const Piece& pc = pieces_[p];
                    if (pc.inst)
                    {
                        if (!instanceRuns.empty() && instanceRuns.back().first + instanceRuns.back().count == pc.inst)
                            instanceRuns.back().count += pc.count;
                        else
                            instanceRuns.push_back({pc.inst, pc.count});
                        instanceCount += pc.count;
                    }
                    else
                        geometry.push_back({pc.firstIndex, pc.count, pc.base});
                }
                if (o.type == RenderOp::Draw)
                    o.geometryCount = (std::uint32_t)geometry.size() - o.geometryFirst;
                continue;
            }
            if (o.type == RenderOp::FxBatch)
                o.instStart = (std::uint32_t)instances.size();
            else if (o.type == RenderOp::Draw)
                o.idxOffset = (std::uint32_t)indices.size();
            for (std::uint32_t p = opFirst_[k]; p != kNoPiece; p = pieces_[p].next)
            {
                const Piece& pc = pieces_[p];
                if (pc.inst)
                    instances.insert(instances.end(), pc.inst, pc.inst + pc.count);
                else
                {
                    const std::size_t at = indices.size();
                    indices.resize(at + pc.count);
                    std::uint32_t* out = indices.data() + at;
                    for (std::uint32_t i = 0; i < pc.count; ++i)
                        out[i] = pc.idx[i] + pc.base;
                }
            }
        }
        if (anyGlass)
        {
            PlanCaptures();
            if (keyCaptures)
                KeyCaptures();
        }
    }

    // ------------------------------------------------------------------ capture planning
    // Glass must see everything drawn before it, so its backdrop is captured (copy + blur pyramid) right before it
    // draws - but only when something was drawn inside the region it samples since the last capture. The whole
    // frame is known here, so a capture is also stretched over the later glass batches that nothing draws under in
    // between: one capture serves a window and every glass control inside it, instead of one capture per control.
    // Drawing is tracked as a list of rectangles (not one growing box), so unrelated draws elsewhere never force a
    // new capture.
    namespace
    {
        // At most 64 rectangles (past that they merge into one), in place: planning allocates nothing.
        struct DirtyRects
        {
            static constexpr int kMax = 64;
            PxRect rects[kMax];
            int count = 0;
            void Clear() { count = 0; }
            void Add(const PxRect& r)
            {
                if (r.Empty())
                    return;
                if (count >= kMax)
                {
                    PxRect u = rects[0];
                    for (int i = 1; i < count; ++i)
                        u = u.Union(rects[i]);
                    rects[0] = u;
                    count = 1;
                }
                rects[count++] = r;
            }
            bool Overlaps(const PxRect& r) const
            {
                for (int i = 0; i < count; ++i)
                    if (rects[i].Overlaps(r))
                        return true;
                return false;
            }
        };

        // What was drawn since a capture, split by kind: glass sees ordinary content anywhere in its blur footprint,
        // but other glass only where it actually sits on it (a glass button on a glass card).
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
                case RenderOp::Callback:
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

        // KeyCaptures' hash: 8-byte words mixed in (a multiply-xorshift; not cryptographic - a key that matched by
        // chance would show a stale frost for a frame).
        inline std::uint64_t Mix(std::uint64_t h, std::uint64_t v)
        {
            h ^= v + 0x9E3779B97F4A7C15ull + (h << 6) + (h >> 2);
            h *= 0xBF58476D1CE4E5B9ull;
            return h ^ (h >> 31);
        }
        std::uint64_t MixBytes(std::uint64_t h, const void* data, std::size_t n)
        {
            const auto* p = static_cast<const unsigned char*>(data);
            for (; n >= 8; n -= 8, p += 8)
            {
                std::uint64_t v;
                std::memcpy(&v, p, 8);
                h = Mix(h, v);
            }
            std::uint64_t v = 0;
            std::memcpy(&v, p, n);
            return Mix(h, v ^ (std::uint64_t)n);
        }
        std::uint64_t MixRect(std::uint64_t h, const PxRect& r) { return MixBytes(h, &r, sizeof(r)); }
        // an instance whose pixels change with the shader's clock alone: a shimmer, a light streak without a clock of
        // its own, a user effect (it may read gTime)
        bool Animated(const fx::Instance& in)
        {
            const std::uint32_t f = in.flags[0];
            return (f & fx::kCustom) || ((f & fx::kShimmer) && in.misc[3] != 0.0f) || ((f & fx::kCaustic) && in.fill1[0] < 0.0f);
        }
        constexpr std::size_t kMaxKeyedBytes = 256 * 1024;   // an op with more data than this is not hashed: no reuse
        int LevelsOf(const RenderOp& op) { return op.blurPx < 0.0f ? kBackdropLevels - 1 : LevelsForBlur(op.blurPx); }
    }

    // Temporal reuse of captures (the renderer, RenderParams::targetStatic): a capture whose region, pyramid and what
    // was drawn under it are the same as last frame's needs no new copy and blur - last frame's are still in the
    // pyramid. Each capture's key mixes its region and pyramid with every op before it that touches the region, in
    // order (their kind, state and data); when one of them reads the backdrop itself (glass, a user effect) or is a
    // glow layer, what it shows depends on more than the region: then every op before the capture is mixed in.
    void FramePlan::KeyCaptures()
    {
        opHash_.assign(ops.size(), 0);
        for (std::size_t i = 0; i < ops.size(); ++i)
        {
            RenderOp& c = ops[i];
            if (c.type != RenderOp::FxBatch || c.captureRegion.Empty())
                continue;
            std::uint64_t shape = MixRect(Mix(0x51ED270B27BB09ADull, (std::uint64_t)c.captureLevels), c.captureRegion);
            shape = MixRect(Mix(shape, c.captureLevel0 ? 1u : 0u), c.captureLevel0Region);
            c.captureShape = shape;
            if (!keyCaptureContent)
            {
                c.captureKey = 0;
                continue;
            }
            bool whole = false;
            for (std::size_t k = 0; k < i && !whole; ++k)
            {
                const RenderOp& o = ops[k];
                const PxRect touched = o.type == RenderOp::Callback ? o.clip : o.bounds;
                whole = (o.type == RenderOp::LayerBegin || o.type == RenderOp::LayerEnd || (o.type == RenderOp::FxBatch && o.glass)) &&
                        (touched.Empty() || touched.Overlaps(c.captureRegion));
            }
            std::uint64_t key = shape;
            for (std::size_t k = 0; k < i; ++k)
            {
                const RenderOp& o = ops[k];
                const PxRect touched = o.type == RenderOp::Callback ? o.clip : o.bounds;
                if (!whole && !touched.Overlaps(c.captureRegion))
                    continue;
                const std::uint64_t h = OpHash(k);
                if (h == 0)
                {
                    key = 0;
                    break;
                }
                key = Mix(key, h);
            }
            c.captureKey = key;
        }
    }

    // An op's hash: its kind, state and data (0: it cannot be told, see RenderOp::captureKey).
    std::uint64_t FramePlan::OpHash(std::size_t k)
    {
        if (opHash_[k] != 0)
            return opHash_[k] == 1 ? 0 : opHash_[k];
        const RenderOp& o = ops[k];
        std::uint64_t h = Mix(0x2545F4914F6CDD1Dull, (std::uint64_t)o.type);
        bool known = o.type != RenderOp::Callback;
        h = Mix(h, ((std::uint64_t)o.texture << 32) | o.effect);
        h = Mix(h, ((std::uint64_t)o.features << 8) | (o.quads ? 1u : 0u) | (o.coverage ? 2u : 0u) | (o.lcd ? 4u : 0u) | (o.glass ? 8u : 0u));
        h = MixRect(h, o.clip);
        h = MixBytes(h, o.fade, sizeof(o.fade));
        if (o.type == RenderOp::LayerBegin || o.type == RenderOp::LayerEnd)
            h = MixBytes(MixRect(h, o.bounds), &o.layer, sizeof(o.layer));
        std::size_t bytes = 0;
        for (std::uint32_t p = k < opFirst_.size() ? opFirst_[k] : kNoPiece; known && p != kNoPiece; p = pieces_[p].next)
        {
            const Piece& pc = pieces_[p];
            if (pc.inst)
            {
                for (std::uint32_t i = 0; i < pc.count && known; ++i)
                    known = !Animated(pc.inst[i]);
                bytes += (std::size_t)pc.count * sizeof(fx::Instance);
                if (known && bytes <= kMaxKeyedBytes)
                    h = MixBytes(h, pc.inst, (std::size_t)pc.count * sizeof(fx::Instance));
            }
            else if (pc.quad)
            {
                bytes += (std::size_t)pc.count * sizeof(QuadInstance);
                if (bytes <= kMaxKeyedBytes)
                    h = MixBytes(h, pc.quad, (std::size_t)pc.count * sizeof(QuadInstance));
            }
            else if (pc.idx)
            {
                // the triangles' vertices, through the indices: the list's (a direct plan), or the plan's copy
                const Vertex* vtx = nullptr;
                if (!direct)
                    vtx = vertices.data() + pc.base;
                else
                    for (const ListGeometry& l : lists)
                        if (l.firstVertex == pc.base)
                            vtx = l.list->Vertices().data();
                bytes += (std::size_t)pc.count * (sizeof(Vertex) + 4);
                known = known && vtx != nullptr;
                for (std::uint32_t i = 0; known && i < pc.count && bytes <= kMaxKeyedBytes; ++i)
                    h = MixBytes(h, &vtx[pc.idx[i]], sizeof(Vertex));
            }
            known = known && bytes <= kMaxKeyedBytes;
        }
        h = known ? (h <= 1 ? h + 2 : h) : 1;
        opHash_[k] = h;
        return h == 1 ? 0 : h;
    }

    void FramePlan::PlanCaptures()
    {
        plannedCaptures = 0;
        bool valid = false;
        PxRect validRegion;
        int validLevels = 0;
        PxRect validLevel0;   // where the last capture copied level 0
        Drawn drawn;   // since the last capture
        int depth = 0;
        const int n = (int)ops.size();
        for (int i = 0; i < n; ++i)
        {
            RenderOp& op = ops[(std::size_t)i];
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
                    if (!(valid && validRegion.Contains(op.glassRegion) && validLevels >= levels &&
                          (!op.readsLevel0 || validLevel0.Contains(op.level0Region)) && !drawn.Hides(op)))
                    {
                        PxRect region = op.glassRegion;
                        int lv = levels;
                        PxRect l0 = op.level0Region;
                        float areaSum = Area(region);
                        Drawn ahead;
                        ahead.Add(op);
                        int d = depth;
                        for (int k = i + 1; k < n && k < i + 512; ++k)
                        {
                            const RenderOp& q = ops[(std::size_t)k];
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
                                l0 = l0.Union(q.level0Region);
                            }
                            if (d == 0 && (q.type == RenderOp::Draw || q.type == RenderOp::FxBatch || q.type == RenderOp::Callback))
                                ahead.Add(q);
                        }
                        op.captureRegion = region;
                        op.captureLevels = lv;
                        op.captureLevel0 = !l0.Empty();
                        op.captureLevel0Region = l0;
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
            case RenderOp::Callback:
                if (depth == 0)
                    drawn.Add(op);
                break;
            default:
                break;
            }
        }
    }
}
