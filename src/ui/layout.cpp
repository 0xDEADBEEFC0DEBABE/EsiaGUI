// WGT UI - auto layout: responsive containers (stacks, adaptive grid, flow), the per-window item map and
// effect-aware spacing (glows never cover neighbouring items).
//
// How it works
//   * ImGui::ItemSize() reports every laid-out item to WgtOnItemLaidOut() (and EndGroup lays a group out
//     at its parent's depth). Inside a container, an item submitted at
//     the container's group depth is a *child*: the hook records its size, then moves the cursor and the
//     content region to the next child's slot. Nested containers are groups, i.e. single children.
//   * Slots are computed in Begin*() from what the children measured last frame (size, flexibility, glow
//     overflow) and the width available now, so the layout follows the page size every frame. The first
//     frame of a container only measures (it is clipped away), so nothing is ever shown in a wrong place.
//   * Children glide to new slots with a spring. Positions are animated relative to the container origin,
//     so moving or scrolling a window never lags.
//   * The same hook feeds a per-window item map: Painter uses it to fade an outer glow out before it
//     reaches a neighbouring item (ComputeGlowHalo), and glowing children get extra room in containers.
#include "ui/ui_internal.hpp"

namespace wgt
{
    namespace
    {
        float AlignFactor(std::uint8_t a)
        {
            switch ((ui::Align)a)
            {
            case ui::Align::Center: return 0.5f;
            case ui::Align::End: return 1.0f;
            default: return 0.0f;
            }
        }

        float FlexOf(const LayoutChild& c) { return c.flex > 0.0f ? c.flex : (c.fill ? 1.0f : 0.0f); }
        float Gap(float spacing, const LayoutChild& a, const LayoutChild& b) { return std::max(spacing, a.overflow + b.overflow); }

        bool Overlaps(const Rect& a, const Rect& b) { return a.min.x < b.max.x && a.max.x > b.min.x && a.min.y < b.max.y && a.max.y > b.min.y; }

        // ------------------------------------------------------------ arrangement
        // Pure functions: children (measured sizes / flags) + container width -> offered region and aligned
        // position of every child, and the container size.
        void ArrangeColumn(const LayoutFrame& L, const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size)
        {
            const int n = (int)k.size();
            const float W = L.width;
            const Vec2 o = L.origin;
            float y = n > 0 ? k[0].overflow * 0.5f : 0.0f;
            float maxW = 0.0f;
            bool anyFill = false;
            for (int i = 0; i < n; ++i)
            {
                if (i > 0)
                    y += Gap(L.spacing, k[i - 1], k[i]);
                const float w = k[i].fill ? W : std::min(k[i].size.x, W);
                const float x = k[i].fill ? 0.0f : (W - w) * AlignFactor(L.align);
                regions[i] = Rect(o.x, o.y + y, o.x + W, o.y + y + k[i].size.y);
                places[i] = Vec2(o.x + x, o.y + y);
                y += k[i].size.y;
                maxW = std::max(maxW, w);
                anyFill |= k[i].fill;
            }
            if (n > 0)
                y += k[n - 1].overflow * 0.5f;
            size = Vec2((anyFill || (ui::Align)L.align != ui::Align::Start) ? W : maxW, y);
        }

        void ArrangeRow(const LayoutFrame& L, const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size)
        {
            const int n = (int)k.size();
            const float W = L.width;
            const Vec2 o = L.origin;
            float fixed = 0.0f, flexSum = 0.0f, H = 0.0f, gaps = 0.0f, over = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                const float f = FlexOf(k[i]);
                if (f <= 0.0f)
                    fixed += k[i].size.x;
                flexSum += f;
                H = std::max(H, k[i].size.y);
                over = std::max(over, k[i].overflow);
                if (i > 0)
                    gaps += Gap(L.spacing, k[i - 1], k[i]);
            }
            const float pad = over * 0.5f;   // glows above / below the row
            const float free = std::max(0.0f, W - fixed - gaps);
            float x = 0.0f, between = 0.0f;
            if (flexSum <= 0.0f)
            {
                switch ((ui::Align)L.justify)
                {
                case ui::Align::Center: x = free * 0.5f; break;
                case ui::Align::End: x = free; break;
                case ui::Align::Stretch: between = n > 1 ? free / (float)(n - 1) : 0.0f; break;
                default: break;
                }
            }
            for (int i = 0; i < n; ++i)
            {
                if (i > 0)
                    x += Gap(L.spacing, k[i - 1], k[i]) + between;
                const float f = FlexOf(k[i]);
                const float w = f > 0.0f ? free * f / flexSum : k[i].size.x;
                const float yo = (H - k[i].size.y) * AlignFactor(L.align);
                regions[i] = Rect(o.x + x, o.y + pad, o.x + x + w, o.y + pad + H);
                places[i] = Vec2(o.x + x, o.y + pad + yo);
                x += w;
            }
            size = Vec2((flexSum > 0.0f || (ui::Align)L.justify != ui::Align::Start) ? W : x, H + pad * 2.0f);
        }

        void ArrangeGrid(const LayoutFrame& L, const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size)
        {
            const int n = (int)k.size();
            const float W = L.width;
            const Vec2 o = L.origin;
            const float s = L.spacing;
            int cols = L.columns > 0 ? L.columns : std::max(1, (int)std::floor((W + s) / (std::max(L.minColumn, 1.0f) + s)));
            if (L.maxColumns > 0)
                cols = std::min(cols, L.maxColumns);
            const float cellW = std::max(0.0f, (W - s * (float)(cols - 1)) / (float)cols);

            std::vector<int> rowOf(n), colOf(n), spanOf(n);
            int row = 0, col = 0;
            for (int i = 0; i < n; ++i)
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
            const int rows = n > 0 ? row + 1 : 0;
            std::vector<float> rowH(rows, 0.0f), rowM(rows, 0.0f), rowY(rows, 0.0f);
            for (int i = 0; i < n; ++i)
            {
                float h = k[i].size.y;
                if (L.cellHeight > 0.0f)
                    h = L.cellHeight;
                else if (L.aspect > 0.0f)
                    h = cellW / L.aspect;
                rowH[rowOf[i]] = std::max(rowH[rowOf[i]], h);
                rowM[rowOf[i]] = std::max(rowM[rowOf[i]], k[i].overflow);
            }
            float y = rows > 0 ? rowM[0] * 0.5f : 0.0f;
            for (int r = 0; r < rows; ++r)
            {
                if (r > 0)
                    y += std::max(L.lineSpacing, rowM[r - 1] + rowM[r]);
                rowY[r] = y;
                y += rowH[r];
            }
            if (rows > 0)
                y += rowM[rows - 1] * 0.5f;
            const float a = AlignFactor(L.align);
            for (int i = 0; i < n; ++i)
            {
                const float x = colOf[i] * (cellW + s);
                const float w = cellW * spanOf[i] + s * (float)(spanOf[i] - 1);
                const Rect cell(o.x + x, o.y + rowY[rowOf[i]], o.x + x + w, o.y + rowY[rowOf[i]] + rowH[rowOf[i]]);
                regions[i] = cell;
                if (k[i].fill || (ui::Align)L.align == ui::Align::Stretch)
                    places[i] = cell.min;
                else
                    places[i] = Vec2(cell.min.x + (w - std::min(k[i].size.x, w)) * a, cell.min.y + std::max(0.0f, cell.Height() - k[i].size.y) * a);
            }
            size = Vec2(W, y);
        }

        void ArrangeFlow(const LayoutFrame& L, const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size)
        {
            struct Line
            {
                int first = 0, last = -1;
                float width = 0.0f, height = 0.0f, margin = 0.0f;
            };
            const int n = (int)k.size();
            const float W = L.width;
            const Vec2 o = L.origin;
            std::vector<Line> lines;
            std::vector<float> xs(n);
            Line cur;
            float x = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                const float w = k[i].fill ? W : std::min(k[i].size.x, W);
                float gap = cur.last >= cur.first ? Gap(L.spacing, k[i - 1], k[i]) : 0.0f;
                if (cur.last >= cur.first && x + gap + w > W + 0.5f)
                {
                    cur.width = x;
                    lines.push_back(cur);
                    cur = Line();
                    cur.first = i;
                    x = 0.0f;
                    gap = 0.0f;
                }
                xs[i] = x + gap;
                x += gap + w;
                cur.last = i;
                cur.height = std::max(cur.height, k[i].size.y);
                cur.margin = std::max(cur.margin, k[i].overflow);
            }
            if (n > 0)
            {
                cur.width = x;
                lines.push_back(cur);
            }
            float y = lines.empty() ? 0.0f : lines[0].margin * 0.5f;
            for (size_t li = 0; li < lines.size(); ++li)
            {
                const Line& ln = lines[li];
                if (li > 0)
                    y += std::max(L.lineSpacing, lines[li - 1].margin + ln.margin);
                const float free = std::max(0.0f, W - ln.width);
                const int count = ln.last - ln.first + 1;
                float lead = 0.0f, between = 0.0f;
                switch ((ui::Align)L.justify)
                {
                case ui::Align::Center: lead = free * 0.5f; break;
                case ui::Align::End: lead = free; break;
                case ui::Align::Stretch: between = count > 1 ? free / (float)(count - 1) : 0.0f; break;
                default: break;
                }
                for (int i = ln.first; i <= ln.last; ++i)
                {
                    const float px = xs[i] + lead + between * (float)(i - ln.first);
                    const float w = k[i].fill ? W : std::min(k[i].size.x, W);
                    regions[i] = Rect(o.x + px, o.y + y, o.x + px + w, o.y + y + ln.height);
                    places[i] = Vec2(o.x + px, o.y + y + (ln.height - k[i].size.y) * AlignFactor(L.align));
                }
                y += ln.height;
            }
            if (!lines.empty())
                y += lines.back().margin * 0.5f;
            size = Vec2(W, y);
        }

        void Arrange(const LayoutFrame& L, const std::vector<LayoutChild>& k, std::vector<Rect>& regions, std::vector<Vec2>& places, Vec2& size)
        {
            regions.resize(k.size());
            places.resize(k.size());
            size = Vec2(0, 0);
            switch (L.kind)
            {
            case LayoutKind::VStack: ArrangeColumn(L, k, regions, places, size); break;
            case LayoutKind::HStack: ArrangeRow(L, k, regions, places, size); break;
            case LayoutKind::Adaptive:
                if (L.horizontal)
                    ArrangeRow(L, k, regions, places, size);
                else
                    ArrangeColumn(L, k, regions, places, size);
                break;
            case LayoutKind::Grid: ArrangeGrid(L, k, regions, places, size); break;
            case LayoutKind::Flow: ArrangeFlow(L, k, regions, places, size); break;
            }
        }

        // Adaptive stack: a row while every child fits at its natural width (flexible ones at `minFill`).
        bool FitsInRow(const LayoutFrame& L, const std::vector<LayoutChild>& k, bool wasRow)
        {
            float need = 0.0f;
            for (size_t i = 0; i < k.size(); ++i)
            {
                need += FlexOf(k[i]) > 0.0f ? L.minFill : k[i].size.x;
                if (i > 0)
                    need += Gap(L.spacing, k[i - 1], k[i]);
            }
            return need <= L.width + (wasRow ? 0.5f : -8.0f);   // hysteresis: no flicker at the threshold
        }

        LayoutFrame* Top(Context::Impl& m) { return m.ui.layouts.empty() ? nullptr : m.ui.layouts.back().get(); }

        // Innermost container, when the item being submitted belongs to its current child.
        LayoutFrame* Current(Context::Impl& m)
        {
            LayoutFrame* L = Top(m);
            ImGuiContext* g = ImGui::GetCurrentContext();
            if (!L || !g || g->CurrentWindow != L->window || g->GroupStack.Size < L->depth)
                return nullptr;
            return L;
        }

        // Moves the cursor and the content region to child `i`'s slot.
        // How far `reach` sticks out of `item` on its worst side (0 when the item contains it).
        float PokeOut(const Rect& item, const Rect& reach)
        {
            return std::max({0.0f, item.min.x - reach.min.x, item.min.y - reach.min.y, reach.max.x - item.max.x, reach.max.y - item.max.y});
        }

        void BeginChildSlot(Context::Impl& m, LayoutFrame& L, int i)
        {
            Rect region;
            Vec2 place;
            const bool known = i < (int)L.regions.size();
            if (known)
            {
                // whole pixels: a settled spring then rounds to the same pixel every frame (a target on a half
                // pixel would flicker between two)
                region = L.regions[i];
                place = L.places[i];
                const Vec2 snapped(std::floor(place.x + 0.5f), std::floor(place.y + 0.5f));
                region = Rect(region.min + (snapped - place), region.max + (snapped - place));
                place = snapped;
            }
            else
            {
                // not measured yet (first frame, or a new child): full-width row below everything so far
                float y = L.origin.y;
                for (const LayoutChild& c : L.now)
                    y = std::max(y, c.pos.y + c.size.y + L.spacing);
                region = Rect(L.origin.x, y, L.origin.x + L.width, y);
                place = region.min;
            }
            Vec2 pos = place;
            if (known && L.animate && !L.hidden)
            {
                static const Spring kReflow{0.30f, 0.90f};
                const Vec2 rel = place - L.origin;
                const std::uint32_t k = 0x4C00u + (std::uint32_t)i * 2u;
                pos = L.origin + Vec2(anim::Float(ui::detail::Salt(L.id, k), rel.x, &kReflow), anim::Float(ui::detail::Salt(L.id, k + 1), rel.y, &kReflow));
                pos = Vec2(std::floor(pos.x + 0.5f), std::floor(pos.y + 0.5f));
            }
            const Vec2 d = pos - place;
            L.region = Rect(region.min + d, region.max + d);

            ImGuiWindow* w = L.window;
            w->DC.CursorPos = pos;
            w->DC.CursorPosPrevLine = pos;
            w->DC.IsSameLine = false;
            w->DC.CurrLineSize = ImVec2(0.0f, 0.0f);
            w->DC.CurrLineTextBaseOffset = 0.0f;
            const float right = std::max(L.region.max.x, pos.x + 1.0f);
            w->WorkRect.Max.x = right;
            w->ContentRegionRect.Max.x = right;

            L.pendingFlex = 0.0f;
            L.pendingSpan = 1;
            L.pendingFill = false;
            L.pendingOverflow = 0.0f;
            L.hasPendingGlow = false;
            (void)m;
        }

        struct LayoutParams
        {
            float spacing = -1.0f, lineSpacing = -1.0f;
            ui::Align align = ui::Align::Start, justify = ui::Align::Start;
            bool animate = true;
            float minColumn = 0.0f, cellHeight = 0.0f, aspect = 0.0f, minFill = 120.0f;
            int columns = 0, maxColumns = 0;
        };

        bool BeginLayout(LayoutKind kind, const char* strId, const LayoutParams& p)
        {
            ImGuiWindow* w = ImGui::GetCurrentWindow();
            if (w->SkipItems)
                return false;
            Context::Impl& m = RequireImpl();
            const float scale = m.theme.current.metrics.scale;
            const float spacing = p.spacing >= 0.0f ? p.spacing * scale : m.theme.current.metrics.spacing * scale;

            auto frame = std::make_unique<LayoutFrame>();
            LayoutFrame& L = *frame;
            L.id = w->GetID(strId);
            L.kind = kind;
            L.window = w;
            L.origin = w->DC.CursorPos;
            L.width = std::max(ui::detail::AvailWidth(), 1.0f);   // also: this container stretches in its parent
            L.spacing = spacing;
            L.lineSpacing = p.lineSpacing >= 0.0f ? p.lineSpacing * scale : spacing;
            L.align = (std::uint8_t)p.align;
            L.justify = (std::uint8_t)p.justify;
            L.animate = p.animate;
            L.minColumn = p.minColumn * scale;
            L.cellHeight = p.cellHeight * scale;
            L.aspect = p.aspect;
            L.minFill = p.minFill * scale;
            L.columns = p.columns;
            L.maxColumns = p.maxColumns;

            LayoutCache& cache = m.ui.layoutCache[L.id];
            cache.lastFrame = m.frameIndex;
            L.cache = &cache;
            L.hidden = !cache.measured;
            L.horizontal = kind == LayoutKind::Adaptive ? FitsInRow(L, cache.children, cache.horizontal) : kind != LayoutKind::VStack;
            Vec2 size;
            Arrange(L, cache.children, L.regions, L.places, size);
            L.now.reserve(cache.children.size() + 4);

            ImGui::BeginGroup();
            L.depth = ImGui::GetCurrentContext()->GroupStack.Size;
            L.backupWorkMaxX = w->WorkRect.Max.x;
            L.backupContentMaxX = w->ContentRegionRect.Max.x;
            if (L.hidden)
                ImGui::PushClipRect(L.origin, L.origin, true);   // measure-only frame: every item is clipped away
            m.ui.layouts.push_back(std::move(frame));
            BeginChildSlot(m, *m.ui.layouts.back(), 0);
            return true;
        }

        void EndLayout()
        {
            Context::Impl& m = RequireImpl();
            IM_ASSERT(!m.ui.layouts.empty() && "EndStack/EndGrid/EndFlow without a matching Begin");
            if (m.ui.layouts.empty())
                return;
            std::unique_ptr<LayoutFrame> frame = std::move(m.ui.layouts.back());
            m.ui.layouts.pop_back();
            LayoutFrame& L = *frame;
            ImGuiWindow* w = L.window;
            ImGuiContext& g = *ImGui::GetCurrentContext();
            IM_ASSERT(g.CurrentWindow == w && "layout container closed in another window");
            if (L.hidden)
                ImGui::PopClipRect();

            // size from this frame's measurements (next frame places with them)
            std::vector<Rect> regions;
            std::vector<Vec2> places;
            Vec2 size;
            Arrange(L, L.now, regions, places, size);
            float overflow = 0.0f;
            for (const LayoutChild& c : L.now)
                overflow = std::max(overflow, c.overflow);
            L.cache->children = std::move(L.now);
            L.cache->measured = true;
            L.cache->horizontal = L.horizontal;

            w->WorkRect.Max.x = L.backupWorkMaxX;
            w->ContentRegionRect.Max.x = L.backupContentMaxX;
            // the group is exactly the arranged size (not the animated / measured extents)
            w->DC.CursorMaxPos = ImMax(L.origin + size, L.origin);
            g.LastItemData.Rect = ImRect(L.origin, L.origin);
            w->DC.IsSetPos = false;
            // glow margins of a row are not absorbed by it: let the parent container see them
            if (LayoutFrame* parent = Top(m))
                if (parent->window == w && (L.kind == LayoutKind::HStack || (L.kind == LayoutKind::Adaptive && L.horizontal)))
                    parent->pendingOverflow = std::max(parent->pendingOverflow, overflow * 0.5f);
            ImGui::EndGroup();   // lays the container out as one item of its parent (WgtOnItemLaidOut)
        }
    }

    // ================================================================= hooks
    namespace ui::detail
    {
        void OnItemSize(Context::Impl& m, ImGuiWindow* window, const Rect& r)
        {
            // per-window item map (glow halos, layout inspector): only the visible part of the item counts
            const ImRect clip = window->ClipRect;
            const Rect vis(ImMax(r.min, clip.Min), ImMin(r.max, clip.Max));
            if (vis.Width() >= 1.0f && vis.Height() >= 1.0f)
            {
                WindowItems& wi = m.ui.items[window->ID];
                if (wi.frame != m.frameIndex)
                {
                    std::swap(wi.prev, wi.cur);
                    wi.cur.clear();
                    wi.curDepth.clear();
                    wi.frame = m.frameIndex;
                    wi.window = window;
                }
                wi.cur.push_back(vis);
                // a floating bar overlaps the content that scrolls under it by design: it gets a depth of its own
                wi.curDepth.push_back(m.ui.floatingItem ? -1 : ImGui::GetCurrentContext()->GroupStack.Size);
            }
            m.ui.floatingItem = false;
            // layout inspector: an item that keeps running past the window's right edge is cut off, content the
            // user cannot see. Items only passing through (page slides, a container's measuring frame) don't
            // count: the same rect has to stay there for half a second.
            if (m.debugLayout.load(std::memory_order_relaxed) && !window->ScrollbarX)   // a horizontal scroller shows its overflow
            {
                // the window's own clip edge (InnerClipRect is also clipped to the screen: a window partly off
                // screen is not a layout problem)
                const float edge = window->InnerRect.Max.x - ImMax(ImTrunc(window->WindowPadding.x * 0.5f), window->WindowBorderSize);
                if (r.max.x > edge + 1.0f && r.Width() >= 1.0f && r.Height() >= 1.0f)
                {
                    const std::uint64_t key = ((std::uint64_t)window->ID << 32) ^ (std::uint64_t)ImHashData(&r, sizeof(r));
                    Context::Impl::CutOffItem& c = m.cutOffItems[key];
                    c.frames = c.lastFrame + 1 == m.frameIndex ? c.frames + 1 : 1;
                    c.lastFrame = m.frameIndex;
                    if (c.frames >= 30)
                    {
                        Painter p(ImGui::GetForegroundDrawList());
                        p.Rect(r, Style().Stroke(2.0f, Color(1.0f, 0.55f, 0.0f, 1.0f), 1.0f));
                        if (!c.reported)
                        {
                            c.reported = true;
                            m.Log(2, "WGT layout: item cut off by the right edge of '%s' (%.0f px past it): [%.0f,%.0f %.0fx%.0f]", window->Name,
                                  r.max.x - edge, r.min.x, r.min.y, r.Width(), r.Height());
                        }
                    }
                }
            }
            // auto-layout child
            LayoutFrame* L = Top(m);
            ImGuiContext* g = ImGui::GetCurrentContext();
            if (!L || L->window != window || g->GroupStack.Size != L->depth)
                return;
            LayoutChild c;
            c.size = r.Size();
            c.pos = r.min;
            c.flex = L->pendingFlex;
            c.span = L->pendingSpan;
            c.fill = L->pendingFill;
            c.overflow = L->pendingOverflow;
            if (L->hasPendingGlow)
                c.overflow = std::max(c.overflow, PokeOut(r, L->pendingGlow));
            L->now.push_back(c);
            BeginChildSlot(m, *L, (int)L->now.size());
        }

        float AvailWidth()
        {
            if (Context::Impl* m = CurrentImpl())
                if (LayoutFrame* L = Current(*m))
                    L->pendingFill = true;
            return ImGui::GetContentRegionAvail().x;
        }

        bool InLayoutContainer()
        {
            Context::Impl* m = CurrentImpl();
            return m && Current(*m) != nullptr;
        }
    }

    void LayoutNewFrame(Context::Impl& m)
    {
        if (!m.ui.layouts.empty())
        {
            m.Log(2, "WGT: %d layout container(s) were not closed last frame (missing EndStack/EndGrid/EndFlow)", (int)m.ui.layouts.size());
            m.ui.layouts.clear();
        }
        if ((m.frameIndex % 240) == 0)
        {
            for (auto it = m.ui.layoutCache.begin(); it != m.ui.layoutCache.end();)
                it = (m.frameIndex - it->second.lastFrame > 600) ? m.ui.layoutCache.erase(it) : std::next(it);
            for (auto it = m.ui.items.begin(); it != m.ui.items.end();)
                it = (m.frameIndex - it->second.frame > 600) ? m.ui.items.erase(it) : std::next(it);
            for (auto it = m.cutOffItems.begin(); it != m.cutOffItems.end();)
                it = (m.frameIndex - it->second.lastFrame > 600) ? m.cutOffItems.erase(it) : std::next(it);
        }
    }

    void LayoutInspect(Context::Impl& m)
    {
        if (!m.debugLayout.load())
            return;
        ImDrawList* fg = ImGui::GetForegroundDrawList();
        for (auto& [id, wi] : m.ui.items)
        {
            if (wi.frame != m.frameIndex || !wi.window || !wi.window->WasActive)
                continue;
            const size_t n = wi.cur.size();
            for (size_t i = 0; i < n; ++i)
                for (size_t j = i + 1; j < n; ++j)
                {
                    if (wi.curDepth[i] != wi.curDepth[j])
                        continue;   // a parent and its children overlap by design
                    const Rect& a = wi.cur[i];
                    const Rect& b = wi.cur[j];
                    const float ix = std::min(a.max.x, b.max.x) - std::max(a.min.x, b.min.x);
                    const float iy = std::min(a.max.y, b.max.y) - std::max(a.min.y, b.min.y);
                    if (ix <= 1.5f || iy <= 1.5f)
                        continue;   // touching / sub-pixel rounding
                    const bool aInB = a.min.x >= b.min.x - 0.5f && a.min.y >= b.min.y - 0.5f && a.max.x <= b.max.x + 0.5f && a.max.y <= b.max.y + 0.5f;
                    const bool bInA = b.min.x >= a.min.x - 0.5f && b.min.y >= a.min.y - 0.5f && b.max.x <= a.max.x + 0.5f && b.max.y <= a.max.y + 0.5f;
                    if (aInB || bInA)
                        continue;   // background / reserved-space pattern
                    Painter p(fg);
                    const Style outline = Style().Stroke(2.0f, Color(1.0f, 0.16f, 0.16f, 1.0f), 1.0f);
                    p.Rect(a, outline);
                    p.Rect(b, outline);
                    const std::uint64_t key = ((std::uint64_t)id << 32) ^ ((std::uint64_t)i << 16) ^ (std::uint64_t)j;
                    if (!m.reportedOverlaps[key])
                    {
                        m.reportedOverlaps[key] = true;
                        m.Log(2, "WGT layout: overlapping items in '%s': [%.0f,%.0f %.0fx%.0f] and [%.0f,%.0f %.0fx%.0f]", wi.window->Name, a.min.x, a.min.y,
                              a.Width(), a.Height(), b.min.x, b.min.y, b.Width(), b.Height());
                    }
                }
        }
    }

    bool ComputeGlowHalo(Context::Impl& m, const ImDrawList* dl, const Rect& shape, float extent, Rect& bound, float& fade)
    {
        ImGuiContext* g = ImGui::GetCurrentContext();
        ImGuiWindow* w = g ? g->CurrentWindow : nullptr;
        if (!w || w->DrawList != dl)
            return false;
        auto found = m.ui.items.find(w->ID);
        if (found == m.ui.items.end())
            return false;
        // last frame's complete item list (this frame's list is still being built)
        const std::vector<Rect>& items = found->second.frame == m.frameIndex ? found->second.prev : found->second.cur;
        const Rect self = shape.Expanded(0.5f);
        bound = shape.Expanded(extent);
        bool constrained = false;
        float minGap = extent;
        for (const Rect& o : items)
        {
            if (!Overlaps(o, bound) || Overlaps(o, self))   // out of reach, or the shape's own item / container
                continue;
            const float gapT = shape.min.y - o.max.y, gapB = o.min.y - shape.max.y;
            const float gapL = shape.min.x - o.max.x, gapR = o.min.x - shape.max.x;
            const bool xOverlap = o.max.x > shape.min.x && o.min.x < shape.max.x;
            const bool yOverlap = o.max.y > shape.min.y && o.min.y < shape.max.y;
            bool vertical;
            if (xOverlap)
                vertical = true;
            else if (yOverlap)
                vertical = false;
            else
                vertical = std::max(gapT, gapB) >= std::max(gapL, gapR);   // diagonal: cut the roomier axis
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
        bound = Rect(ImMin(bound.min, shape.min), ImMax(bound.max, shape.max));
        fade = std::clamp(minGap * 0.85f, 1.0f, extent);
        return true;
    }

    // A glow reaching `margin` beyond `shape` needs room only where it leaves the layout child it belongs to:
    // a glowing button is its own child (full margin), a glowing toggle inside a tile is usually contained (none).
    void LayoutReportOverflow(Context::Impl& m, const ImDrawList* dl, const Rect& shape, float margin)
    {
        LayoutFrame* L = Top(m);
        if (!L || L->window->DrawList != dl || margin <= 0.0f)
            return;
        const Rect reach = shape.Expanded(margin);
        for (int i = (int)L->now.size() - 1; i >= 0; --i)
        {
            LayoutChild& c = L->now[i];
            const Rect item(c.pos, c.pos + c.size);
            if (Overlaps(item.Expanded(1.0f), shape))
            {
                c.overflow = std::max(c.overflow, PokeOut(item, reach));
                return;
            }
        }
        // drawn inside a child that is still open (a group): measured against it when it is laid out
        L->pendingGlow = L->hasPendingGlow ? L->pendingGlow.Union(reach) : reach;
        L->hasPendingGlow = true;
    }

    // ============================================================ public API
    namespace ui
    {
        bool BeginVStack(const char* id, const StackOptions& o)
        {
            LayoutParams p;
            p.spacing = o.spacing;
            p.align = o.align;
            p.justify = o.justify;
            p.animate = o.animate;
            return BeginLayout(LayoutKind::VStack, id, p);
        }

        bool BeginHStack(const char* id, const StackOptions& o)
        {
            LayoutParams p;
            p.spacing = o.spacing;
            p.align = o.align;
            p.justify = o.justify;
            p.animate = o.animate;
            return BeginLayout(LayoutKind::HStack, id, p);
        }

        bool BeginAdaptiveStack(const char* id, const StackOptions& o)
        {
            LayoutParams p;
            p.spacing = o.spacing;
            p.align = o.align;
            p.justify = o.justify;
            p.animate = o.animate;
            p.minFill = o.minFillWidth;
            return BeginLayout(LayoutKind::Adaptive, id, p);
        }

        void EndStack() { EndLayout(); }

        bool BeginGrid(const char* id, const GridOptions& o)
        {
            LayoutParams p;
            p.spacing = o.spacing;
            p.lineSpacing = o.rowSpacing;
            p.align = o.align;
            p.animate = o.animate;
            p.minColumn = o.minColumnWidth;
            p.columns = o.columns;
            p.maxColumns = o.maxColumns;
            p.cellHeight = o.cellHeight;
            p.aspect = o.aspect;
            return BeginLayout(LayoutKind::Grid, id, p);
        }

        void EndGrid() { EndLayout(); }

        bool BeginFlow(const char* id, const FlowOptions& o)
        {
            LayoutParams p;
            p.spacing = o.spacing;
            p.lineSpacing = o.lineSpacing;
            p.align = o.align;
            p.justify = o.justify;
            p.animate = o.animate;
            return BeginLayout(LayoutKind::Flow, id, p);
        }

        void EndFlow() { EndLayout(); }

        void LayoutFlex(float flex)
        {
            if (LayoutFrame* L = Current(RequireImpl()))
                L->pendingFlex = std::max(0.0f, flex);
        }

        void LayoutSpan(int columns)
        {
            if (LayoutFrame* L = Current(RequireImpl()))
                L->pendingSpan = std::max(1, columns);
        }

        void FlexSpacer()
        {
            LayoutFlex(1.0f);
            ImGui::Dummy(Vec2(0.0f, 0.0f));
        }

        SizeClass GetSizeClass(float width)
        {
            const float w = width >= 0.0f ? width : ImGui::GetContentRegionAvail().x;
            const float design = w / std::max(RequireImpl().theme.current.metrics.scale, 0.01f);
            return design < 420.0f ? SizeClass::Compact : (design < 760.0f ? SizeClass::Regular : SizeClass::Expanded);
        }
    }
}

// Called by ImGui::ItemSize() for every laid-out item (declared in imgui_internal.h).
void WgtOnItemLaidOut(ImGuiWindow* window, const ImRect& bb)
{
    wgt::Context::Impl* m = wgt::CurrentImpl();
    if (m && m->inFrame && window)
        wgt::ui::detail::OnItemSize(*m, window, wgt::Rect(bb.Min, bb.Max));
}
