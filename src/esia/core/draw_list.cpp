// Esia - draw list implementation (see esia/core/draw_list.hpp)
#include "esia/core/draw_list.hpp"
#include <algorithm>
#include <cstring>
#if defined(__SSE2__) || defined(_M_X64)
#include <emmintrin.h>
#endif

namespace esia
{
    void DrawList::Reset(const Rect& clip)
    {
        vtx_.clear();
        idx_.clear();
        quads_.clear();
        cmds_.clear();
        fx_.clear();
        layers_.clear();
        fades_.clear();
        clipStack_.assign(1, clip);
        textureStack_.clear();
        mergeBarrier_ = 0;
        rec_ = RecordState();
    }

    void DrawList::PushClipRect(const Rect& r, bool intersect)
    {
        Rect c = intersect ? r.Intersect(clipStack_.back()) : r;
        // an inverted rect would make culling and scissors disagree: normalize to empty
        if (c.max.x < c.min.x)
            c.max.x = c.min.x;
        if (c.max.y < c.min.y)
            c.max.y = c.min.y;
        clipStack_.push_back(c);
    }

    void DrawList::PopClipRect()
    {
        ESIA_ASSERT(clipStack_.size() > 1 && "PopClipRect without PushClipRect");
        if (rec_.active && clipStack_.size() <= rec_.clipDepth)
            rec_.broken = true;   // a clip from before the recording
        if (clipStack_.size() > 1)
            clipStack_.pop_back();
    }

    void DrawList::PushTexture(TextureId texture) { textureStack_.push_back(texture); }

    void DrawList::PopTexture()
    {
        ESIA_ASSERT(!textureStack_.empty() && "PopTexture without PushTexture");
        if (rec_.active && textureStack_.size() <= rec_.textureDepth)
            rec_.broken = true;
        if (!textureStack_.empty())
            textureStack_.pop_back();
    }

    DrawCmd& DrawList::Push(DrawCmdKind kind)
    {
        DrawCmd& c = cmds_.emplace_back();
        c.kind = kind;
        c.clip = clipStack_.back();
        return c;
    }

    DrawCmd& DrawList::Geometry(TextureId tex)
    {
        if (cmds_.size() > mergeBarrier_)
        {
            DrawCmd& last = cmds_.back();
            if (last.kind == DrawCmdKind::Geometry && last.texture == tex && last.clip == clipStack_.back() &&
                last.first + last.count == (std::uint32_t)idx_.size())
                return last;
            // an empty geometry command left behind by a state change is reused instead of piling up
            if (last.kind == DrawCmdKind::Geometry && last.count == 0)
            {
                last.texture = tex;
                last.clip = clipStack_.back();
                last.first = (std::uint32_t)idx_.size();
                last.vtxFirst = last.vtxEnd = (std::uint32_t)vtx_.size();
                last.vtxBounds = DrawCmd().vtxBounds;
                return last;
            }
        }
        DrawCmd& c = Push(DrawCmdKind::Geometry);
        c.texture = tex;
        c.first = (std::uint32_t)idx_.size();
        c.vtxFirst = c.vtxEnd = (std::uint32_t)vtx_.size();
        return c;
    }

    std::uint32_t DrawList::PrimBegin(std::uint32_t indexCount, std::uint32_t vertexCount)
    {
        Geometry();
        Reserve(indexCount, vertexCount);
        return (std::uint32_t)vtx_.size();
    }

    void DrawList::Reserve(std::uint32_t indexCount, std::uint32_t vertexCount)
    {
        // grown geometrically: reserve(size + n) allocates exactly that, so a frame that outgrew the last one copied
        // the whole list at every primitive
        if (idx_.capacity() < idx_.size() + indexCount)
            idx_.reserve(std::max(idx_.size() + indexCount, idx_.capacity() * 2));
        if (vtx_.capacity() < vtx_.size() + vertexCount)
            vtx_.reserve(std::max(vtx_.size() + vertexCount, vtx_.capacity() * 2));
    }

    void DrawList::AddRectFilledUV(const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
    {
        if (r.Empty() || (color >> 24) == 0)
            return;
        // written in place (a glyph is one of these: text is most of what a frame adds)
        const std::uint32_t b = PrimBegin(6, 4);
        const std::size_t v = vtx_.size(), i = idx_.size();
        vtx_.resize(v + 4);
        idx_.resize(i + 6);
        Vertex* w = vtx_.data() + v;
        w[0] = {r.min, uv0, color};
        w[1] = {Vec2(r.max.x, r.min.y), Vec2(uv1.x, uv0.y), color};
        w[2] = {r.max, uv1, color};
        w[3] = {Vec2(r.min.x, r.max.y), Vec2(uv0.x, uv1.y), color};
        std::uint32_t* x = idx_.data() + i;
        x[0] = b;
        x[1] = b + 1;
        x[2] = b + 2;
        x[3] = b;
        x[4] = b + 2;
        x[5] = b + 3;
        DrawCmd& c = cmds_.back();
        c.count += 6;
        c.vtxEnd = (std::uint32_t)vtx_.size();
        c.vtxBounds = c.vtxBounds.Union(r);
    }

    void DrawList::AddRectFilled(const Rect& r, std::uint32_t color) { AddRectFilledUV(r, Vec2(0, 0), Vec2(1, 1), color); }

    DrawCmd& DrawList::QuadCommand(TextureId tex, bool shape)
    {
        if (cmds_.size() > mergeBarrier_)
        {
            DrawCmd& last = cmds_.back();
            if (last.kind == DrawCmdKind::Quads && last.clip == clipStack_.back() && last.first + last.count == (std::uint32_t)quads_.size())
            {
                // a shape reads no texture: it joins quads on any; textured quads join shapes alone and give them theirs
                if (shape)
                    return last;
                if (last.texture == tex || last.anyTexture)
                {
                    last.texture = tex;
                    last.anyTexture = false;
                    return last;
                }
            }
            if (last.kind == DrawCmdKind::Quads && last.count == 0)
            {
                last.texture = tex;
                last.anyTexture = shape;
                last.shapes = false;
                last.clip = clipStack_.back();
                last.first = (std::uint32_t)quads_.size();
                last.vtxBounds = DrawCmd().vtxBounds;
                return last;
            }
        }
        DrawCmd& c = Push(DrawCmdKind::Quads);
        c.texture = tex;
        c.anyTexture = shape;
        c.first = (std::uint32_t)quads_.size();
        return c;
    }

    DrawList::QuadWriter DrawList::BeginQuads(TextureId texture, std::uint32_t maxQuads)
    {
        QuadCommand(texture);
        const std::size_t at = quads_.size();
        if (quads_.capacity() < at + maxQuads)
            quads_.reserve(std::max(at + maxQuads, quads_.capacity() * 2));
        quads_.resize(at + maxQuads);
        QuadWriter w;
        w.quad = quads_.data() + at;
        return w;
    }

    void DrawList::EndQuads(const QuadWriter& w)
    {
        quads_.resize((std::size_t)(w.quad - quads_.data()) + w.written);
        if (w.written > 0)
        {
            DrawCmd& c = cmds_.back();
            c.count += w.written;
            c.vtxBounds = c.vtxBounds.Union(w.bounds);
        }
    }

    void DrawList::AddQuad(TextureId texture, const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
    {
        if (r.Empty() || (color >> 24) == 0)
            return;
        QuadWriter w = BeginQuads(texture, 1);
        w.Add(r, uv0, uv1, color);
        EndQuads(w);
    }

    void DrawList::AddCoverageQuad(TextureId texture, const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
    {
        if (r.Empty() || (color >> 24) == 0)
            return;
        DrawCmd& c = QuadCommand(texture);
        quads_.emplace_back().SetCoverage(r, uv0, uv1, color);
        ++c.count;
        c.shapes = true;   // the programs that tell it from the glyphs
        c.vtxBounds = c.vtxBounds.Union(r);
    }

    void DrawList::AddShapeQuad(const QuadInstance& shape, const Rect& bounds)
    {
        if (shape.Alpha() == 0)
            return;
        DrawCmd& c = QuadCommand(0, true);
        quads_.push_back(shape);
        ++c.count;
        c.shapes = true;
        c.vtxBounds = c.vtxBounds.Union(bounds);
    }

    void DrawList::AddShapeQuads(const QuadInstance* shapes, std::size_t count, const Rect& bounds)
    {
        if (count == 0 || shapes[0].Alpha() == 0)
            return;
        std::memcpy(AppendShapeQuads(count, bounds), shapes, count * sizeof(QuadInstance));
    }

    QuadInstance* DrawList::AppendShapeQuads(std::size_t count, const Rect& bounds)
    {
        DrawCmd& c = QuadCommand(0, true);
        const std::size_t at = quads_.size();
        quads_.resize(at + count);
        c.count += (std::uint32_t)count;
        c.shapes = true;
        c.vtxBounds = c.vtxBounds.Union(bounds);
        return quads_.data() + at;
    }

    DrawList::PrimWriter DrawList::PrimReserve(std::uint32_t indexCount, std::uint32_t vertexCount)
    {
        PrimWriter w;
        w.base = PrimBegin(indexCount, vertexCount);
        const std::size_t i = idx_.size();
        vtx_.resize(w.base + (std::size_t)vertexCount);
        idx_.resize(i + indexCount);
        w.vtx = vtx_.data() + w.base;
        w.idx = idx_.data() + i;
        return w;
    }

    DrawList::PrimWriter DrawList::PrimReserve(TextureId texture, std::uint32_t indexCount, std::uint32_t vertexCount)
    {
        DrawCmd* cmd = &Geometry(texture);
        Reserve(indexCount, vertexCount);
        PrimWriter w;
        w.cmd = cmd;
        w.base = (std::uint32_t)vtx_.size();
        const std::size_t i = idx_.size();
        vtx_.resize(w.base + (std::size_t)vertexCount);
        idx_.resize(i + indexCount);
        w.vtx = vtx_.data() + w.base;
        w.idx = idx_.data() + i;
        return w;
    }

    void DrawList::PrimCommit(std::uint32_t indexCount, const Rect& bounds)
    {
        DrawCmd& c = cmds_.back();
        c.count += indexCount;
        c.vtxEnd = (std::uint32_t)vtx_.size();
        c.vtxBounds = c.vtxBounds.Union(bounds);
    }

    void DrawList::RefreshBounds(std::size_t fromVertex, std::size_t fromQuad)
    {
        for (DrawCmd& c : cmds_)
        {
            Rect b = DrawCmd().vtxBounds;
            if (c.kind == DrawCmdKind::Geometry && c.vtxEnd > fromVertex)
            {
                for (std::uint32_t v = c.vtxFirst; v < c.vtxEnd; ++v)
                    b = b.Union(Rect(vtx_[v].pos, vtx_[v].pos));
            }
            else if (c.kind == DrawCmdKind::Quads && c.first + c.count > fromQuad)
            {
                for (std::uint32_t q = c.first; q < c.first + c.count; ++q)
                    b = b.Union(quads_[q].Bounds());
            }
            else
                continue;
            c.vtxBounds = b;
            if (rec_.active && (std::size_t)(&c - cmds_.data()) < rec_.cmds)
            {
                // a command from before the recording: its bounds are all of it now (after MoveCommands any of them
                // may be the one before it: none is given back then)
                rec_.broken = true;
                rec_.refreshed = rec_.refreshed || (std::size_t)(&c - cmds_.data()) + 1 == rec_.cmds || rec_.restored;
            }
        }
    }

    void DrawList::AddImage(TextureId texture, const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
    {
        PushTexture(texture);
        AddRectFilledUV(r, uv0, uv1, color);
        PopTexture();
    }

    void DrawList::AddTriangleFilled(Vec2 a, Vec2 b, Vec2 c, std::uint32_t color)
    {
        if ((color >> 24) == 0)
            return;
        const std::uint32_t base = PrimBegin(3, 3);
        WriteVertex(a, Vec2(0, 0), color);
        WriteVertex(b, Vec2(0, 0), color);
        WriteVertex(c, Vec2(0, 0), color);
        WriteTriangle(base, base + 1, base + 2);
    }

    void DrawList::AddConvexPolyFilled(const Vec2* points, int count, std::uint32_t color)
    {
        if (count < 3 || (color >> 24) == 0)
            return;
        const std::uint32_t base = PrimBegin((std::uint32_t)(count - 2) * 3, (std::uint32_t)count);
        for (int i = 0; i < count; ++i)
            WriteVertex(points[i], Vec2(0, 0), color);
        for (int i = 2; i < count; ++i)
            WriteTriangle(base, base + (std::uint32_t)i - 1, base + (std::uint32_t)i);
    }

    void DrawList::AddFx(const fx::Instance& instance, EffectId effect, TextureId texture)
    {
        const Rect& clip = clipStack_.back();
        if (cmds_.size() > mergeBarrier_)
        {
            DrawCmd& last = cmds_.back();
            if (last.kind == DrawCmdKind::Fx && last.clip == clip && last.texture == texture && last.effect == effect &&
                last.first + last.count == (std::uint32_t)fx_.size())
            {
                fx_.push_back(instance);
                ++last.count;
                return;
            }
        }
        DrawCmd& c = Push(DrawCmdKind::Fx);
        c.texture = texture;
        c.effect = effect;
        c.first = (std::uint32_t)fx_.size();
        c.count = 1;
        fx_.push_back(instance);
    }

    void DrawList::BeginLayer(const fx::LayerParams& params)
    {
        rec_.broken = rec_.broken || rec_.active;
        DrawCmd& c = Push(DrawCmdKind::LayerBegin);
        c.payload = (std::uint32_t)layers_.size();
        layers_.push_back(params);
    }

    void DrawList::EndLayer()
    {
        rec_.broken = rec_.broken || rec_.active;
        Push(DrawCmdKind::LayerEnd);
    }

    void DrawList::BeginFade(const fx::FadeParams& params)
    {
        rec_.broken = rec_.broken || rec_.active;
        DrawCmd& c = Push(DrawCmdKind::FadeBegin);
        c.payload = (std::uint32_t)fades_.size();
        fades_.push_back(params);
    }

    void DrawList::EndFade()
    {
        rec_.broken = rec_.broken || rec_.active;
        Push(DrawCmdKind::FadeEnd);
    }

    void DrawList::AddCallback(DrawCallback callback, void* userData)
    {
        rec_.broken = rec_.broken || rec_.active;
        DrawCmd& c = Push(DrawCmdKind::Callback);
        c.callback = callback;
        c.userData = userData;
    }

    std::size_t DrawList::Mark()
    {
        rec_.broken = rec_.broken || rec_.active;
        mergeBarrier_ = cmds_.size();
        return cmds_.size();
    }

    void DrawList::MoveCommands(std::size_t from, std::size_t to)
    {
        ESIA_ASSERT(to <= from && from <= cmds_.size() && "MoveCommands: bad range");
        if (to >= from || from > cmds_.size())
            return;
        if (rec_.active)
        {
            rec_.broken = true;
            if (rec_.hadLast && !rec_.restored && !rec_.refreshed)
            {
                // the command before the recording may move: its bounds back first (with what it gathered)
                DrawCmd& last = cmds_[rec_.cmds - 1];
                last.vtxBounds = rec_.before.vtxBounds.Union(last.vtxBounds);
                rec_.restored = true;
            }
        }
        std::rotate(cmds_.begin() + (std::ptrdiff_t)to, cmds_.begin() + (std::ptrdiff_t)from, cmds_.end());
        // the last command is now an earlier one: new primitives start a command of their own
        mergeBarrier_ = cmds_.size();
    }

    // ------------------------------------------------------------------ recording
    void DrawRecord::Clear()
    {
        before = DrawCmd();
        hadLast = mergeOk = contiguous = took = anyTextureAfter = shapesAfter = false;
        clip = Rect();
        texture = textureAfter = 0;
        addCount = 0;
        addBounds = DrawCmd().vtxBounds;
        vtxEnd = -1;
        vtx.clear();   // (the storage stays for the next recording)
        idx.clear();
        quads.clear();
        fx.clear();
        cmds.clear();
    }

    namespace
    {
        // what decides how a primitive joins a command, and what it leaves in it: all but its ranges and bounds
        bool SameState(const DrawCmd& a, const DrawCmd& b)
        {
            return a.kind == b.kind && a.clip == b.clip && a.texture == b.texture && a.anyTexture == b.anyTexture && a.shapes == b.shapes &&
                   a.effect == b.effect && a.payload == b.payload && a.callback == b.callback && a.userData == b.userData;
        }

        bool SameCmd(const DrawCmd& a, const DrawCmd& b)
        {
            return SameState(a, b) && a.first == b.first && a.count == b.count && a.vtxFirst == b.vtxFirst && a.vtxEnd == b.vtxEnd &&
                   a.vtxBounds == b.vtxBounds;
        }

        template <class V>
        bool SameBytes(const V& a, const V& b)
        {
            return a.size() == b.size() && (a.empty() || std::memcmp(a.data(), b.data(), a.size() * sizeof(a[0])) == 0);
        }
    }

    bool DrawRecord::operator==(const DrawRecord& o) const
    {
        if (hadLast != o.hadLast || mergeOk != o.mergeOk || contiguous != o.contiguous || !(clip == o.clip) || texture != o.texture ||
            (hadLast && !SameState(before, o.before)) || took != o.took || cmds.size() != o.cmds.size())
            return false;
        if (took && (addCount != o.addCount || !(addBounds == o.addBounds) || textureAfter != o.textureAfter || anyTextureAfter != o.anyTextureAfter ||
                     shapesAfter != o.shapesAfter || vtxEnd != o.vtxEnd))
            return false;
        for (std::size_t i = 0; i < cmds.size(); ++i)
            if (!SameCmd(cmds[i], o.cmds[i]))
                return false;
        return SameBytes(vtx, o.vtx) && SameBytes(idx, o.idx) && SameBytes(quads, o.quads) && SameBytes(fx, o.fx);
    }

    namespace
    {
        // A widget's few quads copied here (a call to the C runtime's memcpy cost as much as the copy), a long text's
        // with memcpy
        void CopyQuads(QuadInstance* to, const QuadInstance* from, std::size_t n)
        {
            if (n > 24)
            {
                std::memcpy(to, from, n * sizeof(QuadInstance));
                return;
            }
#if defined(__SSE2__) || defined(_M_X64)
            for (std::size_t i = 0; i < n; ++i)
            {
                const __m128 a = _mm_loadu_ps(from[i].rect), b = _mm_loadu_ps(from[i].packed);
                _mm_storeu_ps(to[i].rect, a);
                _mm_storeu_ps(to[i].packed, b);
            }
#else
            for (std::size_t i = 0; i < n; ++i)
                to[i] = from[i];
#endif
        }
    }

    bool DrawList::Contiguous(const DrawCmd& c) const
    {
        switch (c.kind)
        {
        case DrawCmdKind::Geometry: return c.first + c.count == (std::uint32_t)idx_.size();
        case DrawCmdKind::Quads: return c.first + c.count == (std::uint32_t)quads_.size();
        case DrawCmdKind::Fx: return c.first + c.count == (std::uint32_t)fx_.size();
        default: return false;
        }
    }

    void DrawList::BeginRecord()
    {
        ESIA_ASSERT(!rec_.active && "BeginRecord: a recording is going on");
        RecordState& r = rec_;
        r = RecordState();
        r.active = true;
        r.vtx = vtx_.size();
        r.idx = idx_.size();
        r.quads = quads_.size();
        r.fx = fx_.size();
        r.cmds = cmds_.size();
        r.layers = layers_.size();
        r.fades = fades_.size();
        r.clipDepth = clipStack_.size();
        r.textureDepth = textureStack_.size();
        r.mergeBarrier = mergeBarrier_;
        r.hadLast = !cmds_.empty();
        r.mergeOk = cmds_.size() > mergeBarrier_;
        if (r.hadLast)
        {
            DrawCmd& last = cmds_.back();
            r.before = last;
            r.contiguous = Contiguous(last);
            last.vtxBounds = DrawCmd().vtxBounds;   // gathers what the stretch adds to it
        }
    }

    bool DrawList::EndRecord(DrawRecord& out)
    {
        ESIA_ASSERT(rec_.active && "EndRecord without BeginRecord");
        RecordState& r = rec_;
        r.active = false;
        out.Clear();
        bool ok = !r.broken && clipStack_.size() == r.clipDepth && textureStack_.size() == r.textureDepth && mergeBarrier_ == r.mergeBarrier &&
                  layers_.size() == r.layers && fades_.size() == r.fades && cmds_.size() >= r.cmds;
        if (r.hadLast && r.cmds <= cmds_.size())
        {
            DrawCmd& last = cmds_[r.cmds - 1];
            const DrawCmd& b = r.before;
            const Rect added = last.vtxBounds;
            if (!r.refreshed && !r.restored)
                last.vtxBounds = b.vtxBounds.Union(added);   // (refreshed: all of it already, as unrecorded)
            const bool emptyAdded = added == DrawCmd().vtxBounds;
            if (last.count != b.count || !emptyAdded || last.texture != b.texture || last.anyTexture != b.anyTexture || last.shapes != b.shapes ||
                last.vtxEnd != b.vtxEnd)
            {
                // it took primitives: only by joining it (a command left empty is taken over, clip and all: not kept)
                out.took = true;
                out.addCount = last.count - b.count;
                out.addBounds = added;
                out.textureAfter = last.texture;
                out.anyTextureAfter = last.anyTexture;
                out.shapesAfter = last.shapes;
                out.vtxEnd = last.vtxEnd != b.vtxEnd ? (std::int64_t)last.vtxEnd - (std::int64_t)r.vtx : -1;
                if (b.count == 0 || last.count < b.count || last.kind != b.kind || !(last.clip == b.clip) || last.first != b.first ||
                    last.vtxFirst != b.vtxFirst || last.effect != b.effect || out.vtxEnd < -1)
                    ok = false;
            }
        }
        if (!ok)
        {
            out.Clear();
            return false;
        }
        out.before = r.before;
        out.hadLast = r.hadLast;
        out.mergeOk = r.mergeOk;
        out.contiguous = r.contiguous;
        out.clip = clipStack_.back();
        out.texture = CurrentTexture();
        out.vtx.assign(vtx_.begin() + (std::ptrdiff_t)r.vtx, vtx_.end());
        out.idx.resize(idx_.size() - r.idx);
        for (std::size_t i = 0; i < out.idx.size(); ++i)
        {
            if (idx_[r.idx + i] < (std::uint32_t)r.vtx)
            {
                out.Clear();   // a vertex from before the stretch
                return false;
            }
            out.idx[i] = idx_[r.idx + i] - (std::uint32_t)r.vtx;
        }
        out.quads.assign(quads_.begin() + (std::ptrdiff_t)r.quads, quads_.end());
        out.fx.assign(fx_.begin() + (std::ptrdiff_t)r.fx, fx_.end());
        out.cmds.assign(cmds_.begin() + (std::ptrdiff_t)r.cmds, cmds_.end());
        for (DrawCmd& c : out.cmds)
        {
            if (c.kind == DrawCmdKind::Geometry)
            {
                c.first -= (std::uint32_t)r.idx;
                c.vtxFirst -= (std::uint32_t)r.vtx;
                c.vtxEnd -= (std::uint32_t)r.vtx;
            }
            else if (c.kind == DrawCmdKind::Quads)
                c.first -= (std::uint32_t)r.quads;
            else
                c.first -= (std::uint32_t)r.fx;
        }
        return true;
    }

    bool DrawList::CanReplay(const DrawRecord& rec) const
    {
        if (cmds_.empty() == rec.hadLast || (cmds_.size() > mergeBarrier_) != rec.mergeOk || !(clipStack_.back() == rec.clip) ||
            CurrentTexture() != rec.texture)
            return false;
        if (!rec.hadLast)
            return true;
        const DrawCmd& last = cmds_.back();
        return SameState(last, rec.before) && Contiguous(last) == rec.contiguous && (last.count == 0) == (rec.before.count == 0);
    }

    void DrawList::Replay(const DrawRecord& rec)
    {
        ESIA_ASSERT(CanReplay(rec) && "Replay: the list is not in the state the stretch began in");
        const std::uint32_t v0 = (std::uint32_t)vtx_.size(), i0 = (std::uint32_t)idx_.size(), q0 = (std::uint32_t)quads_.size(),
                            f0 = (std::uint32_t)fx_.size();
        if (rec.took)
        {
            DrawCmd& last = cmds_.back();
            last.count += rec.addCount;
            last.vtxBounds = last.vtxBounds.Union(rec.addBounds);
            last.texture = rec.textureAfter;
            last.anyTexture = rec.anyTextureAfter;
            last.shapes = rec.shapesAfter;
            if (rec.vtxEnd >= 0)
                last.vtxEnd = v0 + (std::uint32_t)rec.vtxEnd;
        }
        if (!rec.vtx.empty())
        {
            Reserve((std::uint32_t)rec.idx.size(), (std::uint32_t)rec.vtx.size());
            vtx_.resize(v0 + rec.vtx.size());
            std::memcpy(vtx_.data() + v0, rec.vtx.data(), rec.vtx.size() * sizeof(Vertex));
        }
        if (!rec.idx.empty())
        {
            idx_.resize(i0 + rec.idx.size());
            std::uint32_t* to = idx_.data() + i0;
            for (std::size_t i = 0; i < rec.idx.size(); ++i)
                to[i] = rec.idx[i] + v0;
        }
        if (!rec.quads.empty())
        {
            if (quads_.capacity() < q0 + rec.quads.size())
                quads_.reserve(std::max<std::size_t>(q0 + rec.quads.size(), quads_.capacity() * 2));
            quads_.resize(q0 + rec.quads.size());
            CopyQuads(quads_.data() + q0, rec.quads.data(), rec.quads.size());
        }
        if (!rec.fx.empty())
            fx_.insert(fx_.end(), rec.fx.begin(), rec.fx.end());
        for (DrawCmd c : rec.cmds)
        {
            if (c.kind == DrawCmdKind::Geometry)
            {
                c.first += i0;
                c.vtxFirst += v0;
                c.vtxEnd += v0;
            }
            else if (c.kind == DrawCmdKind::Quads)
                c.first += q0;
            else
                c.first += f0;
            cmds_.push_back(c);
        }
    }

    std::size_t DrawData::TotalVertices() const
    {
        std::size_t n = 0;
        for (const DrawList* l : lists)
            n += l->Vertices().size();
        return n;
    }

    std::size_t DrawData::TotalIndices() const
    {
        std::size_t n = 0;
        for (const DrawList* l : lists)
            n += l->Indices().size();
        return n;
    }

    std::size_t DrawData::TotalQuads() const
    {
        std::size_t n = 0;
        for (const DrawList* l : lists)
            n += l->Quads().size();
        return n;
    }

    std::size_t DrawData::TotalFx() const
    {
        std::size_t n = 0;
        for (const DrawList* l : lists)
            n += l->FxInstances().size();
        return n;
    }
}
