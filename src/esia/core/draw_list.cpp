// Esia - draw list implementation (see esia/core/draw_list.hpp)
#include "esia/core/draw_list.hpp"
#include <algorithm>

namespace esia
{
    void DrawList::Reset(const Rect& clip)
    {
        vtx_.clear();
        idx_.clear();
        cmds_.clear();
        fx_.clear();
        layers_.clear();
        fades_.clear();
        clipStack_.assign(1, clip);
        textureStack_.clear();
        mergeBarrier_ = 0;
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
        if (clipStack_.size() > 1)
            clipStack_.pop_back();
    }

    void DrawList::PushTexture(TextureId texture) { textureStack_.push_back(texture); }

    void DrawList::PopTexture()
    {
        ESIA_ASSERT(!textureStack_.empty() && "PopTexture without PushTexture");
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

    DrawCmd& DrawList::Geometry()
    {
        const TextureId tex = CurrentTexture();
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
                return last;
            }
        }
        DrawCmd& c = Push(DrawCmdKind::Geometry);
        c.texture = tex;
        c.first = (std::uint32_t)idx_.size();
        return c;
    }

    std::uint32_t DrawList::PrimBegin(std::uint32_t indexCount, std::uint32_t vertexCount)
    {
        Geometry();
        idx_.reserve(idx_.size() + indexCount);
        vtx_.reserve(vtx_.size() + vertexCount);
        return (std::uint32_t)vtx_.size();
    }

    void DrawList::AddRectFilledUV(const Rect& r, Vec2 uv0, Vec2 uv1, std::uint32_t color)
    {
        if (r.Empty() || (color >> 24) == 0)
            return;
        const std::uint32_t b = PrimBegin(6, 4);
        WriteVertex(r.min, uv0, color);
        WriteVertex(Vec2(r.max.x, r.min.y), Vec2(uv1.x, uv0.y), color);
        WriteVertex(r.max, uv1, color);
        WriteVertex(Vec2(r.min.x, r.max.y), Vec2(uv0.x, uv1.y), color);
        WriteTriangle(b, b + 1, b + 2);
        WriteTriangle(b, b + 2, b + 3);
    }

    void DrawList::AddRectFilled(const Rect& r, std::uint32_t color) { AddRectFilledUV(r, Vec2(0, 0), Vec2(1, 1), color); }

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
        DrawCmd& c = Push(DrawCmdKind::LayerBegin);
        c.payload = (std::uint32_t)layers_.size();
        layers_.push_back(params);
    }

    void DrawList::EndLayer() { Push(DrawCmdKind::LayerEnd); }

    void DrawList::BeginFade(const fx::FadeParams& params)
    {
        DrawCmd& c = Push(DrawCmdKind::FadeBegin);
        c.payload = (std::uint32_t)fades_.size();
        fades_.push_back(params);
    }

    void DrawList::EndFade() { Push(DrawCmdKind::FadeEnd); }

    void DrawList::AddCallback(DrawCallback callback, void* userData)
    {
        DrawCmd& c = Push(DrawCmdKind::Callback);
        c.callback = callback;
        c.userData = userData;
    }

    std::size_t DrawList::Mark()
    {
        mergeBarrier_ = cmds_.size();
        return cmds_.size();
    }

    void DrawList::MoveCommands(std::size_t from, std::size_t to)
    {
        ESIA_ASSERT(to <= from && from <= cmds_.size() && "MoveCommands: bad range");
        if (to >= from || from > cmds_.size())
            return;
        std::rotate(cmds_.begin() + (std::ptrdiff_t)to, cmds_.begin() + (std::ptrdiff_t)from, cmds_.end());
        // the last command is now an earlier one: new primitives start a command of their own
        mergeBarrier_ = cmds_.size();
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

    std::size_t DrawData::TotalFx() const
    {
        std::size_t n = 0;
        for (const DrawList* l : lists)
            n += l->FxInstances().size();
        return n;
    }
}
