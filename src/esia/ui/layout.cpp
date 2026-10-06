// Esia UI - auto layout: stacks, adaptive stacks, grids and flows as LayoutProviders of the core (WGT's layout.cpp).
//
// A container's children are measured every frame. Begin places them from what they measured last frame and the
// width offered now, so a page re-flows with its window every frame; End measures this frame. The first frame of a
// container only measures (its content is clipped away), so nothing ever shows in a wrong place. Children glide to
// new places on a spring, relative to the container, so moving or scrolling the window never lags.
#include "ui_internal.hpp"
#include <algorithm>

namespace esia::ui
{
    using namespace detail;

    enum class LayoutKind : std::uint8_t { VStack, HStack, Adaptive, Grid, Flow };

    struct LayoutChild
    {
        Vec2 size{0, 0};         // measured size
        Vec2 pos{0, 0};          // where it was placed
        float flex = 0.0f;       // share of the free main-axis space (LayoutFlex)
        int span = 1;            // grid columns (LayoutSpan)
        float overflow = 0.0f;   // visual overflow (glow) it reported
        bool fill = false;       // asked for the available width: stretches to what it is offered
    };

    class BoxLayout final : public LayoutProvider
    {
    public:
        // configuration, set every frame before the container begins
        LayoutKind kind = LayoutKind::VStack;
        float spacing = 0.0f, lineSpacing = 0.0f;
        Align align = Align::Start, justify = Align::Start;
        bool animate = true;
        float minColumn = 0.0f, cellHeight = 0.0f, aspect = 0.0f, minFill = 0.0f;
        int columns = 0, maxColumns = 0;
        // what the next child asked for (LayoutFlex, LayoutSpan, AvailableWidth)
        float pendingFlex = 0.0f;
        int pendingSpan = 1;
        bool pendingFill = false;

        bool Measured() const { return measured_; }

        // Does the container stretch to what its parent offers? Grids and flows lay out in it; a stack only when it
        // spreads its children (a flexible child, or aligned / justified away from the start), else it is as wide
        // as its content and sits next to its siblings (two gauges side by side in a flow). Before its first
        // measurement a stack knows only its options.
        bool WantsFill() const
        {
            if (kind == LayoutKind::Grid || kind == LayoutKind::Flow)
                return true;
            bool flexible = false;
            for (const LayoutChild& c : last_)
                flexible |= FlexOf(c) > 0.0f;
            const bool row = kind == LayoutKind::HStack || (kind == LayoutKind::Adaptive && horizontal_);
            return flexible || (row ? justify != Align::Start : align != Align::Start);
        }

        LayoutSlot Begin(Vec2 origin, const Rect& region) override
        {
            origin_ = origin;
            width_ = std::max(region.max.x - origin.x, 1.0f);
            horizontal_ = kind == LayoutKind::Adaptive ? FitsInRow(last_, horizontal_) : kind != LayoutKind::VStack;
            Vec2 size;
            Arrange(last_, regions_, places_, size);
            now_.clear();
            if (springs_.size() < regions_.size())
                springs_.resize(regions_.size());
            ResetPending();
            return Slot(0);
        }

        LayoutSlot Next(const LaidOutItem& child) override
        {
            LayoutChild c;
            c.size = child.rect.Size();
            c.pos = child.rect.min;
            c.flex = pendingFlex;
            c.span = pendingSpan;
            c.fill = pendingFill;
            if (hasPendingGlow_)
                c.overflow = PokeOut(child.rect, pendingGlow_);   // a glow drawn inside it while it was open
            now_.push_back(c);
            ResetPending();
            return Slot((int)now_.size());
        }

        Vec2 End(float* baseline) override
        {
            // this frame's measurements size the container; next frame places with them (the lists trade places
            // and keep their storage: a frame allocates nothing)
            Vec2 size;
            Arrange(now_, endRegions_, endPlaces_, size);
            last_.swap(now_);
            now_.clear();
            measured_ = true;
            if (baseline)
                *baseline = -1.0f;
            return size;
        }

    private:
        struct SlotSpring
        {
            SpringState x, y;
            bool init = false;
        };

        void ResetPending()
        {
            pendingFlex = 0.0f;
            pendingSpan = 1;
            pendingFill = false;
            hasPendingGlow_ = false;
        }

        // How far `reach` extends past `item` on its farthest side.
        static float PokeOut(const Rect& item, const Rect& reach)
        {
            return std::max({item.min.x - reach.min.x, reach.max.x - item.max.x, item.min.y - reach.min.y, reach.max.y - item.max.y, 0.0f});
        }

    public:
        // A glow reaching `reach` around `shape`, drawn in this container: the child it is drawn on keeps room for it
        // (its overflow widens the gaps next frame); drawn inside a child still open (a nested container), it counts
        // when that child is laid out. A glow that stays inside its child (a toggle in a tile) takes no room.
        void ReportGlow(const Rect& shape, const Rect& reach)
        {
            for (auto it = now_.rbegin(); it != now_.rend(); ++it)
            {
                const Rect item(it->pos, it->pos + it->size);
                if (item.Expanded(1.0f).Overlaps(shape))
                {
                    it->overflow = std::max(it->overflow, PokeOut(item, reach));
                    return;
                }
            }
            pendingGlow_ = hasPendingGlow_ ? pendingGlow_.Union(reach) : reach;
            hasPendingGlow_ = true;
        }

    private:

        static float AlignFactor(Align a) { return a == Align::Center ? 0.5f : a == Align::End ? 1.0f : 0.0f; }
        static float FlexOf(const LayoutChild& c) { return c.flex > 0.0f ? c.flex : (c.fill ? 1.0f : 0.0f); }
        float Gap(const LayoutChild& a, const LayoutChild& b) const { return std::max(spacing, a.overflow + b.overflow); }

        // Where child `i` goes: last frame's arrangement, else (not measured yet) a full-width row below the rest.
        LayoutSlot Slot(int i)
        {
            Rect region;
            Vec2 place;
            const bool known = i < (int)regions_.size();
            if (known)
            {
                // whole pixels: a settled spring then rounds to the same pixel every frame (a target on a half pixel
                // would flicker between two)
                region = regions_[(std::size_t)i];
                place = places_[(std::size_t)i];
                const Vec2 snapped(std::floor(place.x + 0.5f), std::floor(place.y + 0.5f));
                region = region.Translated(snapped - place);
                place = snapped;
            }
            else
            {
                float y = origin_.y;
                for (const LayoutChild& c : now_)
                    y = std::max(y, c.pos.y + c.size.y + spacing);
                region = Rect(origin_.x, y, origin_.x + width_, y);
                place = region.min;
            }
            Vec2 pos = place;
            if (known && animate && measured_)
            {
                static constexpr Spring kReflow{0.30f, 0.90f};
                SlotSpring& s = springs_[(std::size_t)i];
                const Vec2 rel = place - origin_;
                if (!s.init)
                {
                    s.init = true;
                    s.x.value = rel.x;
                    s.y.value = rel.y;
                }
                const float dt = M().dt;
                s.x.Step(rel.x, kReflow, dt);
                s.y.Step(rel.y, kReflow, dt);
                if (s.x.value != rel.x || s.y.value != rel.y)
                    M().animating = true;
                pos = origin_ + Vec2(std::floor(s.x.value + 0.5f), std::floor(s.y.value + 0.5f));
            }
            region = region.Translated(pos - place);
            region.max.x = std::max(region.max.x, pos.x + 1.0f);
            return {pos, region};
        }

        // ---- arrangement: children (measured sizes / flags) + the width -> each child's region and aligned place
        void Arrange(const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size) const
        {
            regions.assign(k.size(), Rect());
            places.assign(k.size(), Vec2());
            size = Vec2(0, 0);
            switch (kind)
            {
            case LayoutKind::VStack: ArrangeColumn(k, regions, places, size); break;
            case LayoutKind::HStack: ArrangeRow(k, regions, places, size); break;
            case LayoutKind::Adaptive:
                if (horizontal_)
                    ArrangeRow(k, regions, places, size);
                else
                    ArrangeColumn(k, regions, places, size);
                break;
            case LayoutKind::Grid: ArrangeGrid(k, regions, places, size); break;
            case LayoutKind::Flow: ArrangeFlow(k, regions, places, size); break;
            }
        }

        void ArrangeColumn(const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size) const
        {
            const std::size_t n = k.size();
            const float W = width_;
            const Vec2 o = origin_;
            float y = n > 0 ? k[0].overflow * 0.5f : 0.0f;
            float maxW = 0.0f;
            bool anyFill = false;
            for (std::size_t i = 0; i < n; ++i)
            {
                if (i > 0)
                    y += Gap(k[i - 1], k[i]);
                const float w = k[i].fill ? W : std::min(k[i].size.x, W);
                const float x = k[i].fill ? 0.0f : (W - w) * AlignFactor(align);
                regions[i] = Rect(o.x, o.y + y, o.x + W, o.y + y + k[i].size.y);
                places[i] = Vec2(o.x + x, o.y + y);
                y += k[i].size.y;
                maxW = std::max(maxW, w);
                anyFill |= k[i].fill;
            }
            if (n > 0)
                y += k[n - 1].overflow * 0.5f;
            size = Vec2((anyFill || align != Align::Start) ? W : maxW, y);
        }

        void ArrangeRow(const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size) const
        {
            const std::size_t n = k.size();
            const float W = width_;
            const Vec2 o = origin_;
            float fixed = 0.0f, flexSum = 0.0f, H = 0.0f, gaps = 0.0f, over = 0.0f;
            for (std::size_t i = 0; i < n; ++i)
            {
                const float f = FlexOf(k[i]);
                if (f <= 0.0f)
                    fixed += k[i].size.x;
                flexSum += f;
                H = std::max(H, k[i].size.y);
                over = std::max(over, k[i].overflow);
                if (i > 0)
                    gaps += Gap(k[i - 1], k[i]);
            }
            const float pad = over * 0.5f;   // glows above and below the row
            const float free = std::max(0.0f, W - fixed - gaps);
            float x = 0.0f, between = 0.0f;
            if (flexSum <= 0.0f)
            {
                switch (justify)
                {
                case Align::Center: x = free * 0.5f; break;
                case Align::End: x = free; break;
                case Align::Stretch: between = n > 1 ? free / (float)(n - 1) : 0.0f; break;
                default: break;
                }
            }
            for (std::size_t i = 0; i < n; ++i)
            {
                if (i > 0)
                    x += Gap(k[i - 1], k[i]) + between;
                const float f = FlexOf(k[i]);
                const float w = f > 0.0f ? free * f / flexSum : k[i].size.x;
                const float yo = (H - k[i].size.y) * AlignFactor(align);
                regions[i] = Rect(o.x + x, o.y + pad, o.x + x + w, o.y + pad + H);
                places[i] = Vec2(o.x + x, o.y + pad + yo);
                x += w;
            }
            size = Vec2((flexSum > 0.0f || justify != Align::Start) ? W : x, H + pad * 2.0f);
        }

        void ArrangeGrid(const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size) const
        {
            const std::size_t n = k.size();
            const float W = width_;
            const Vec2 o = origin_;
            const float s = spacing;
            int cols = columns > 0 ? columns : std::max(1, (int)std::floor((W + s) / (std::max(minColumn, 1.0f) + s)));
            if (maxColumns > 0)
                cols = std::min(cols, maxColumns);
            const float cellW = std::max(0.0f, (W - s * (float)(cols - 1)) / (float)cols);

            std::vector<int>& rowOf = gridInts_[0];
            std::vector<int>& colOf = gridInts_[1];
            std::vector<int>& spanOf = gridInts_[2];
            rowOf.assign(n, 0);
            colOf.assign(n, 0);
            spanOf.assign(n, 0);
            int row = 0, col = 0;
            for (std::size_t i = 0; i < n; ++i)
            {
                const int span = std::clamp(k[i].span, 1, cols);
                if (col + span > cols)
                {
                    ++row;
                    col = 0;
                }
                rowOf[i] = row;
                colOf[i] = col;
                spanOf[i] = span;
                col += span;
            }
            const std::size_t rows = n > 0 ? (std::size_t)row + 1 : 0;
            std::vector<float>& rowH = floats_[0];
            std::vector<float>& rowM = floats_[1];
            std::vector<float>& rowY = floats_[2];
            rowH.assign(rows, 0.0f);
            rowM.assign(rows, 0.0f);
            rowY.assign(rows, 0.0f);
            for (std::size_t i = 0; i < n; ++i)
            {
                float h = k[i].size.y;
                if (cellHeight > 0.0f)
                    h = cellHeight;
                else if (aspect > 0.0f)
                    h = cellW / aspect;
                const std::size_t r = (std::size_t)rowOf[i];
                rowH[r] = std::max(rowH[r], h);
                rowM[r] = std::max(rowM[r], k[i].overflow);
            }
            float y = rows > 0 ? rowM[0] * 0.5f : 0.0f;
            for (std::size_t r = 0; r < rows; ++r)
            {
                if (r > 0)
                    y += std::max(lineSpacing, rowM[r - 1] + rowM[r]);
                rowY[r] = y;
                y += rowH[r];
            }
            if (rows > 0)
                y += rowM[rows - 1] * 0.5f;
            const float a = AlignFactor(align);
            for (std::size_t i = 0; i < n; ++i)
            {
                const std::size_t r = (std::size_t)rowOf[i];
                const float x = (float)colOf[i] * (cellW + s);
                const float w = cellW * (float)spanOf[i] + s * (float)(spanOf[i] - 1);
                const Rect cell(o.x + x, o.y + rowY[r], o.x + x + w, o.y + rowY[r] + rowH[r]);
                regions[i] = cell;
                if (k[i].fill || align == Align::Stretch)
                    places[i] = cell.min;
                else
                    places[i] = Vec2(cell.min.x + (w - std::min(k[i].size.x, w)) * a, cell.min.y + std::max(0.0f, cell.Height() - k[i].size.y) * a);
            }
            size = Vec2(W, y);
        }

        void ArrangeFlow(const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size) const
        {
            const std::size_t n = k.size();
            const float W = width_;
            const Vec2 o = origin_;
            std::vector<FlowLine>& lines = flowLines_;
            lines.clear();
            std::vector<float>& xs = floats_[0];
            xs.assign(n, 0.0f);
            FlowLine cur;
            float x = 0.0f;
            for (std::size_t i = 0; i < n; ++i)
            {
                const float w = k[i].fill ? W : std::min(k[i].size.x, W);
                float gap = cur.count > 0 ? Gap(k[i - 1], k[i]) : 0.0f;
                if (cur.count > 0 && x + gap + w > W + 0.5f)
                {
                    cur.width = x;
                    lines.push_back(cur);
                    cur = FlowLine();
                    cur.first = i;
                    x = 0.0f;
                    gap = 0.0f;
                }
                xs[i] = x + gap;
                x += gap + w;
                ++cur.count;
                cur.height = std::max(cur.height, k[i].size.y);
                cur.margin = std::max(cur.margin, k[i].overflow);
            }
            if (n > 0)
            {
                cur.width = x;
                lines.push_back(cur);
            }
            float y = lines.empty() ? 0.0f : lines[0].margin * 0.5f;
            for (std::size_t li = 0; li < lines.size(); ++li)
            {
                const FlowLine& ln = lines[li];
                if (li > 0)
                    y += std::max(lineSpacing, lines[li - 1].margin + ln.margin);
                const float free = std::max(0.0f, W - ln.width);
                float lead = 0.0f, between = 0.0f;
                switch (justify)
                {
                case Align::Center: lead = free * 0.5f; break;
                case Align::End: lead = free; break;
                case Align::Stretch: between = ln.count > 1 ? free / (float)(ln.count - 1) : 0.0f; break;
                default: break;
                }
                for (std::size_t i = ln.first; i < ln.first + ln.count; ++i)
                {
                    const float px = xs[i] + lead + between * (float)(i - ln.first);
                    const float w = k[i].fill ? W : std::min(k[i].size.x, W);
                    regions[i] = Rect(o.x + px, o.y + y, o.x + px + w, o.y + y + ln.height);
                    places[i] = Vec2(o.x + px, o.y + y + (ln.height - k[i].size.y) * AlignFactor(align));
                }
                y += ln.height;
            }
            if (!lines.empty())
                y += lines.back().margin * 0.5f;
            size = Vec2(W, y);
        }

        // Adaptive stack: a row while every child fits at its natural width (flexible ones at `minFill`).
        bool FitsInRow(const std::vector<LayoutChild>& k, bool wasRow) const
        {
            float need = 0.0f;
            for (std::size_t i = 0; i < k.size(); ++i)
            {
                need += FlexOf(k[i]) > 0.0f ? minFill : k[i].size.x;
                if (i > 0)
                    need += Gap(k[i - 1], k[i]);
            }
            return need <= width_ + (wasRow ? 0.5f : -8.0f);   // hysteresis: no flicker at the threshold
        }

        Vec2 origin_;
        float width_ = 1.0f;
        bool horizontal_ = true;
        bool measured_ = false;
        std::vector<LayoutChild> last_, now_;
        Rect pendingGlow_;
        bool hasPendingGlow_ = false;
        std::vector<Rect> regions_;
        std::vector<Vec2> places_;
        std::vector<SlotSpring> springs_;
        // scratch kept from frame to frame: End's arrangement, the grid's and the flow's working lists
        struct FlowLine
        {
            std::size_t first = 0, count = 0;
            float width = 0.0f, height = 0.0f, margin = 0.0f;
        };
        std::vector<Rect> endRegions_;
        std::vector<Vec2> endPlaces_;
        mutable std::vector<int> gridInts_[3];
        mutable std::vector<float> floats_[3];
        mutable std::vector<FlowLine> flowLines_;
    };

    namespace
    {
        void BeginBox(std::string_view strId, LayoutKind kind, const StackOptions* st, const GridOptions* gr, const FlowOptions* fl)
        {
            Ui::Impl& m = M();
            Context& c = *m.ctx;
            const Theme& t = T();
            const float themeSpacing = Sc(t.metrics.spacing);
            const bool stylePushed = TakeNextStyle();
            const Id id = c.GetId(strId);
            BoxLayout& L = c.State<BoxLayout>(id);
            L.kind = kind;
            if (st)
            {
                L.spacing = st->spacing >= 0.0f ? Sc(st->spacing) : themeSpacing;
                L.lineSpacing = L.spacing;
                L.align = st->align;
                L.justify = st->justify;
                L.minFill = Sc(st->minFillWidth);
                L.animate = st->animate;
            }
            else if (gr)
            {
                L.spacing = gr->spacing >= 0.0f ? Sc(gr->spacing) : themeSpacing;
                L.lineSpacing = gr->rowSpacing >= 0.0f ? Sc(gr->rowSpacing) : L.spacing;
                L.align = gr->align;
                L.minColumn = Sc(gr->minColumnWidth);
                L.columns = gr->columns;
                L.maxColumns = gr->maxColumns;
                L.cellHeight = Sc(gr->cellHeight);
                L.aspect = gr->aspect;
                L.animate = gr->animate;
            }
            else if (fl)
            {
                L.spacing = fl->spacing >= 0.0f ? Sc(fl->spacing) : themeSpacing;
                L.lineSpacing = fl->lineSpacing >= 0.0f ? Sc(fl->lineSpacing) : L.spacing;
                L.justify = fl->justify;
                L.align = fl->align;
                L.animate = fl->animate;
            }
            if (L.WantsFill())
                MarkFill();   // it lays out in the width it is offered, and takes it in its parent
            Ui::Impl::LayoutEntry e;
            e.layout = &L;
            e.list = &c.WindowDrawList();
            e.hidden = !L.Measured();
            const Vec2 origin = c.CursorPos();
            ContainerOptions co;
            co.layout = &L;
            c.BeginContainer(id, co);
            e.depth = c.CurrentDepth();
            if (e.hidden)
                c.PushClipRect(Rect(origin, origin));   // the measuring frame: every item is clipped away
            e.stylePushed = stylePushed;   // the container's style stays for its children until EndBox
            m.layouts.push_back(e);
        }

        void EndBox()
        {
            Ui::Impl& m = M();
            ESIA_ASSERT(!m.layouts.empty() && "EndStack / EndGrid / EndFlow without a matching Begin");
            if (m.layouts.empty())
                return;
            const Ui::Impl::LayoutEntry e = m.layouts.back();
            m.layouts.pop_back();
            if (e.hidden)
                m.ctx->PopClipRect();
            m.ctx->EndContainer();
            if (e.stylePushed)
                PopStyle();
        }

        // the innermost auto-layout container, when the item being submitted is one of its direct children
        BoxLayout* CurrentBox()
        {
            Ui::Impl& m = M();
            if (m.layouts.empty())
                return nullptr;
            const Ui::Impl::LayoutEntry& e = m.layouts.back();
            return m.ctx->CurrentDepth() == e.depth ? e.layout : nullptr;
        }
    }

    bool detail::InLayoutContainer() { return CurrentBox() != nullptr; }

    // Marks the next child of the innermost auto-layout container of this window, also from inside that child (a group,
    // a card): a child with something in it that fills the width fills too (WGT's pending flag).
    void detail::MarkFill()
    {
        Ui::Impl& m = M();
        if (!m.layouts.empty() && m.layouts.back().list == &m.ctx->WindowDrawList())
            m.layouts.back().layout->pendingFill = true;
    }

    namespace
    {
        class ItemMap final : public GlowContainment
        {
        public:
            void ReportGlowReach(const DrawList& dl, const Rect& restShape, float reach) override
            {
                Ui::Impl& m = M();
                if (!m.layouts.empty() && m.layouts.back().list == &dl && reach > 0.0f)
                    m.layouts.back().layout->ReportGlow(restShape, restShape.Expanded(reach));
            }

            // WGT's ComputeGlowHalo: the glow is bounded at the first item in its way on each side (not its own item,
            // nor the containers it is in), and fades out over the gap before it.
            bool GlowBounds(const DrawList& dl, const Rect& shape, float extent, Rect& bound, float& fade) override
            {
                Context& c = *M().ctx;
                const Window* w = c.CurrentWindow();
                if (!w || &w->GetDrawList() != &dl)
                    return false;
                const Rect self = shape.Expanded(0.5f);
                bound = shape.Expanded(extent);
                bool constrained = false;
                float minGap = extent;
                for (const LaidOutItem& item : w->LaidOutItems())
                {
                    const Rect& o = item.rect;
                    if (!o.Overlaps(bound) || o.Overlaps(self))
                        continue;
                    const float gapT = shape.min.y - o.max.y, gapB = o.min.y - shape.max.y;
                    const float gapL = shape.min.x - o.max.x, gapR = o.min.x - shape.max.x;
                    const bool xOverlap = o.max.x > shape.min.x && o.min.x < shape.max.x;
                    const bool yOverlap = o.max.y > shape.min.y && o.min.y < shape.max.y;
                    // above / below, beside, or diagonal: cut the roomier axis
                    const bool vertical = xOverlap ? true : yOverlap ? false : std::max(gapT, gapB) >= std::max(gapL, gapR);
                    float gap;
                    if (vertical && gapT >= 0.0f)
                    {
                        bound.min.y = std::max(bound.min.y, o.max.y);
                        gap = gapT;
                    }
                    else if (vertical)
                    {
                        bound.max.y = std::min(bound.max.y, o.min.y);
                        gap = gapB;
                    }
                    else if (gapL >= 0.0f)
                    {
                        bound.min.x = std::max(bound.min.x, o.max.x);
                        gap = gapL;
                    }
                    else
                    {
                        bound.max.x = std::min(bound.max.x, o.min.x);
                        gap = gapR;
                    }
                    constrained = true;
                    minGap = std::min(minGap, std::max(gap, 0.0f));
                }
                if (!constrained)
                    return false;
                bound = bound.Union(shape);
                fade = std::clamp(minGap * 0.85f, 1.0f, extent);
                return true;
            }
        };
    }

    GlowContainment* detail::ItemMapGlow()
    {
        static ItemMap map;   // stateless: it reads the current Ui
        return &map;
    }

    void BeginVStack(std::string_view id, const StackOptions& o) { BeginBox(id, LayoutKind::VStack, &o, nullptr, nullptr); }
    void BeginHStack(std::string_view id, const StackOptions& o) { BeginBox(id, LayoutKind::HStack, &o, nullptr, nullptr); }
    void BeginAdaptiveStack(std::string_view id, const StackOptions& o) { BeginBox(id, LayoutKind::Adaptive, &o, nullptr, nullptr); }
    void EndStack() { EndBox(); }
    void BeginGrid(std::string_view id, const GridOptions& o) { BeginBox(id, LayoutKind::Grid, nullptr, &o, nullptr); }
    void EndGrid() { EndBox(); }
    void BeginFlow(std::string_view id, const FlowOptions& o) { BeginBox(id, LayoutKind::Flow, nullptr, nullptr, &o); }
    void EndFlow() { EndBox(); }

    void LayoutFlex(float flex)
    {
        if (BoxLayout* L = CurrentBox())
            L->pendingFlex = std::max(flex, 0.0f);
    }

    void LayoutSpan(int columns)
    {
        if (BoxLayout* L = CurrentBox())
            L->pendingSpan = std::max(columns, 1);
    }

    void FlexSpacer()
    {
        LayoutFlex(1.0f);
        Ctx().ItemSize(Vec2(0, 0));
    }
}
