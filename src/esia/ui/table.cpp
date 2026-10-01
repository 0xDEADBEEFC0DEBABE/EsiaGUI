// Esia UI - data views: tables (sortable, resizable, selectable, only the rows in view submitted) and trees (outline
// rows whose children slide open and closed).
#include "ui_internal.hpp"
#include <algorithm>
#include <optional>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
        // ============================================================ tables
        struct TableState
        {
            int sortColumn = -1;
            bool ascending = true;
            std::vector<float> widths;   // columns resized by hand (UI units, unscaled), 0 = the column's own
            int anchor = -1;             // where a Shift range starts
            int resizing = -1;           // the column whose right edge is dragged
            float resizeStart = 0.0f, resizeWidth = 0.0f;
        };

        struct TableFrame
        {
            Id id = 0;
            TableOptions options;
            std::vector<TableColumn> columns;
            std::vector<float> x;        // column edges (absolute), columns + 1
            TableSort sort;
            Rect outer;                  // the whole table
            float headerH = 0.0f, rowH = 0.0f;
            float bodyTop = 0.0f;        // where row 0 starts (absolute, scrolled)
            double scroll = 0.0;         // the body's scroll (inner scrolling)
            Rect view;                   // what of the body is visible
            bool scrolls = false;        // rows scroll inside the table
            Id child = 0;
            bool edgeFade = false;
            int rowCount = 0;            // TableVisible's
            int rowsSeen = 0;            // past the last row submitted
            int row = -1, column = -1;
            float rowY = 0.0f;
            bool cellOpen = false;       // a cell container of TableNextColumn
            bool stylePushed = false;
        };
        thread_local std::vector<TableFrame> g_tables;

        TableFrame& CurrentTable()
        {
            ESIA_ASSERT(!g_tables.empty() && "a table call outside BeginTable / EndTable");
            return g_tables.back();
        }

        void CloseCell(TableFrame& t)
        {
            if (!t.cellOpen)
                return;
            Context& c = Ctx();
            c.EndContainer();
            c.PopClipRect();
            t.cellOpen = false;
        }

        Rect CellRect(const TableFrame& t, int column)
        {
            return Rect(t.x[(std::size_t)column], t.rowY, t.x[(std::size_t)column + 1], t.rowY + t.rowH);
        }

        // ============================================================= trees
        struct TreeState
        {
            bool open = false;
            bool known = false;          // the default was applied
            float childHeight = 0.0f;    // the children's height last frame
        };
        struct TreeFrame
        {
            Id id = 0;
            float top = 0.0f;            // where the children start
            float x = 0.0f;              // the guide line
            float shown = 1.0f;          // 0 closed .. 1 open (animated)
            bool clipped = false;        // sliding: clipped to what is shown
        };
        thread_local std::vector<TreeFrame> g_trees;
        thread_local std::optional<bool> g_nextTreeOpen;

        // A chevron ">" turned by `angle` (0 = pointing right, pi/2 = down).
        void Chevron(Painter& p, Vec2 center, float size, float angle, Color color)
        {
            const float cs = std::cos(angle), sn = std::sin(angle);
            const auto rot = [&](float x, float y) { return Vec2(center.x + x * cs - y * sn, center.y + x * sn + y * cs); };
            const float a = size * 0.28f, b = size * 0.5f;
            const Vec2 pts[3] = {rot(-a * 0.8f, -b), rot(a * 1.0f, 0.0f), rot(-a * 0.8f, b)};
            p.Polyline(pts, 3, std::max(1.2f, size * 0.15f), Style().Fill(color));
        }
    }

    // ================================================================ tables
    bool BeginTable(std::string_view id, std::span<const TableColumn> columns, const TableOptions& o)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& th = T();
        const Palette& pc = th.colors;
        if (columns.empty())
            return false;
        TableFrame t;
        t.stylePushed = TakeNextStyle();
        t.id = c.GetId(id);
        t.options = o;
        t.columns.assign(columns.begin(), columns.end());
        TableState& st = c.State<TableState>(t.id);
        st.widths.resize(columns.size(), 0.0f);
        const InputState& in = c.Input();

        const bool header = !(o.flags & TableFlags_NoHeader);
        t.headerH = header ? Sc(30) : 0.0f;
        t.rowH = Sc(o.rowHeight > 0.0f ? o.rowHeight : 34.0f);
        const float width = AvailableWidth();
        const Vec2 pos = c.CursorPos();
        t.scrolls = o.height > 0.0f;
        // a fixed height, or as tall as the header and the rows (fitted at EndTable)
        t.outer = Rect::FromSize(pos, Vec2(width, t.scrolls ? Sc(o.height) : t.headerH));
        ContainerOptions co;
        co.size = Vec2(width, t.scrolls ? Sc(o.height) : 0.0f);
        c.BeginContainer(t.id, co);

        // ---- columns: the fixed ones first, the rest shares what is left by weight
        const float padX = Sc(10);
        float fixed = 0.0f, weights = 0.0f;
        for (std::size_t i = 0; i < columns.size(); ++i)
        {
            const float w = st.widths[i] > 0.0f ? st.widths[i] : columns[i].width;
            if (w > 0.0f)
                fixed += Sc(w);
            else
                weights += std::max(columns[i].weight, 0.01f);
        }
        const float rest = std::max(width - fixed, 0.0f);
        t.x.resize(columns.size() + 1);
        t.x[0] = t.outer.min.x;
        for (std::size_t i = 0; i < columns.size(); ++i)
        {
            const float w = st.widths[i] > 0.0f ? st.widths[i] : columns[i].width;
            const float cw = w > 0.0f ? Sc(w) : std::max(Sc(40), rest * std::max(columns[i].weight, 0.01f) / std::max(weights, 0.01f));
            t.x[i + 1] = t.x[i] + cw;
        }

        // ---- sorting
        t.sort.column = st.sortColumn;
        t.sort.ascending = st.ascending;
        Painter p = GetPainter();
        if (header)
        {
            const Rect hr = Rect::FromSize(pos, Vec2(width, t.headerH));
            p.Rect(hr, Style().Radius(Sc(9)).Fill(pc.fill.Fade(0.55f)));
            const text::FontRef hf = Font(FontWeight::Semibold, th.type.size[(int)TextStyle::Footnote]);
            for (std::size_t i = 0; i < columns.size(); ++i)
            {
                const Rect cell(t.x[i], hr.min.y, t.x[i + 1], hr.max.y);
                const bool sortable = (o.flags & TableFlags_Sortable) && columns[i].sortable;
                const bool sorted = st.sortColumn == (int)i;
                if (sortable)
                {
                    const Interaction it = InteractImpl(Salt(t.id, 0x100 + (std::uint32_t)i), cell.Expanded(-Sc(4), 0.0f), InteractFlags_None);
                    if (it.hover > 0.01f)
                        p.Rect(cell.Expanded(-Sc(2), -Sc(3)), Style().Radius(Sc(7)).Fill(pc.highlight.Fade(it.hover * 0.8f)));
                    if (it.pressed)
                    {
                        if (sorted)
                            st.ascending = !st.ascending;
                        else
                        {
                            st.sortColumn = (int)i;
                            st.ascending = true;
                        }
                        t.sort.column = st.sortColumn;
                        t.sort.ascending = st.ascending;
                        t.sort.changed = true;
                    }
                }
                const std::string_view label = VisibleLabel(columns[i].label);
                const float arrowW = sorted ? Sc(16) : 0.0f;
                const Rect tr(cell.min.x + padX, cell.min.y, cell.max.x - padX - arrowW, cell.max.y);
                const Vec2 ls = MeasureText(hf, label);
                const float ax = columns[i].align == Align::End ? 1.0f : columns[i].align == Align::Center ? 0.5f : 0.0f;
                if (tr.Width() > 0.0f)
                    p.TextBox(Rect(tr.min.x, std::floor(cell.Center().y - ls.y * 0.5f + 0.5f), tr.max.x, std::floor(cell.Center().y - ls.y * 0.5f + 0.5f) + ls.y),
                              Vec2(ax, 0.0f), hf, sorted ? pc.label : pc.secondaryLabel, label, text::TextFlags_Ellipsis);
                if (sorted)
                {
                    const float turn = Anim(t.id, 0x200 + (std::uint32_t)i, st.ascending ? 0.0f : 1.0f, SpringStd());
                    Chevron(p, Vec2(cell.max.x - padX - Sc(5), cell.Center().y), Sc(10), -kPi * 0.5f + kPi * turn, Accent());
                }
            }
            // ---- column edges that drag
            if (o.flags & TableFlags_Resizable)
                for (std::size_t i = 0; i + 1 < columns.size(); ++i)   // the last edge is the table's
                {
                    const float ex = t.x[i + 1];
                    const Rect grip(ex - Sc(4), hr.min.y, ex + Sc(4), hr.max.y);
                    const Interaction gi = InteractImpl(Salt(t.id, 0x300 + (std::uint32_t)i), grip, InteractFlags_PressOnClick);
                    if (gi.pressed)
                    {
                        st.resizing = (int)i;
                        st.resizeStart = in.MousePos().x;
                        st.resizeWidth = (t.x[i + 1] - t.x[i]) / th.metrics.scale;
                    }
                    if (st.resizing == (int)i)
                    {
                        if (gi.held)
                            st.widths[i] = std::max(30.0f, st.resizeWidth + (in.MousePos().x - st.resizeStart) / th.metrics.scale);
                        else
                            st.resizing = -1;
                    }
                    if (gi.hovered || gi.held)
                        c.SetMouseCursor(MouseCursor::ResizeEW);
                    if (gi.hover > 0.01f || gi.held)
                        p.FillRect(Rect(ex - Sc(1), hr.min.y + Sc(6), ex + Sc(1), hr.max.y - Sc(6)), Accent().Fade(std::max(gi.hover, gi.held ? 1.0f : 0.0f)));
                    else
                        p.FillRect(Rect(ex - 0.5f, hr.min.y + Sc(9), ex + 0.5f, hr.max.y - Sc(9)), pc.separator);
                }
        }

        // ---- the body: rows scroll inside it (a fixed height) or are the table's own height
        if (t.scrolls)
        {
            const Rect body(t.outer.min.x, t.outer.min.y + t.headerH, t.outer.max.x, t.outer.max.y);
            c.SetCursorPos(body.min);
            ChildOptions ch;
            ch.size = body.Size();
            ch.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
            t.child = c.GetId("##rows");
            ScrollBegin(t.child);
            c.BeginChild("##rows", ch);
            t.edgeFade = BeginScrollEdgeFade();
            t.view = c.ViewRect().Intersect(c.WindowDrawList().ClipRect());
            t.scroll = c.Scroll().y;
            t.bodyTop = c.ViewRect().min.y;
        }
        else
        {
            // the rows run down from the header; what of them shows is what the clip shows
            const float top = t.outer.min.y + t.headerH;
            t.view = Rect(t.outer.min.x, top, t.outer.max.x, 1e30f).Intersect(c.WindowDrawList().ClipRect());
            t.scroll = 0.0;
            t.bodyTop = top;
        }
        g_tables.push_back(std::move(t));
        return true;
    }

    TableSort TableSortSpec() { return CurrentTable().sort; }

    TableRange TableVisible(int rowCount)
    {
        TableFrame& t = CurrentTable();
        t.rowCount = std::max(rowCount, 0);
        // rows in view, from the visible part of the body (the clip) and the scroll, in double: a long table's
        // offsets are past float's whole numbers
        const double top = (double)(t.view.min.y - t.bodyTop) + t.scroll;
        const double bottom = top + (double)t.view.Height();
        TableRange r;
        r.first = std::clamp((int)std::floor(top / t.rowH), 0, t.rowCount);
        r.last = std::clamp((int)std::ceil(bottom / t.rowH) + 1, r.first, t.rowCount);
        return r;
    }

    TableRowResult TableRow(int index)
    {
        TableFrame& t = CurrentTable();
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Palette& pc = C();
        CloseCell(t);
        t.row = index;
        t.column = -1;
        t.rowsSeen = std::max(t.rowsSeen, index + 1);
        t.rowY = t.bodyTop + (float)((double)index * t.rowH - t.scroll);
        const Rect rr(t.outer.min.x, t.rowY, t.outer.max.x, t.rowY + t.rowH);
        TableRowResult res;
        std::vector<int>* sel = t.options.selection;
        res.selected = sel && std::binary_search(sel->begin(), sel->end(), index);

        // the row is hit below its cells' widgets
        const Id rid = Salt(t.id, 0x10000 + (std::uint32_t)index);
        ButtonResult b;
        if (c.ItemAdd(rid, rr, ItemFlags_Background))
            b = c.ButtonBehavior(rid, rr, ButtonFlags_None, ItemFlags_Background);
        res.clicked = b.pressed;
        res.doubleClicked = b.pressed && b.clicks >= 2;
        if (b.pressed && (t.options.flags & TableFlags_Selectable) && sel)
        {
            TableState& st = c.State<TableState>(t.id);
            const std::uint32_t mods = c.Input().Mods();
            if ((mods & Mod_Shift) && st.anchor >= 0)
            {
                const int a = std::min(st.anchor, index), z = std::max(st.anchor, index);
                if (!(mods & Mod_Ctrl))
                    sel->clear();
                for (int i = a; i <= z; ++i)
                    sel->push_back(i);
            }
            else if (mods & (Mod_Ctrl | Mod_Super))
            {
                auto it = std::lower_bound(sel->begin(), sel->end(), index);
                if (it != sel->end() && *it == index)
                    sel->erase(it);
                else
                    sel->insert(it, index);
                st.anchor = index;
            }
            else
            {
                sel->assign(1, index);
                st.anchor = index;
            }
            std::sort(sel->begin(), sel->end());
            sel->erase(std::unique(sel->begin(), sel->end()), sel->end());
            res.selected = std::binary_search(sel->begin(), sel->end(), index);
        }

        Painter p = GetPainter();
        if ((t.options.flags & TableFlags_Striped) && (index & 1))
            p.FillRect(rr, pc.fill.Fade(0.28f));
        const float hover = Anim(rid, 0xA1, b.hovered ? 1.0f : 0.0f, SpringFast());
        const float selT = Anim(rid, 0x5E1, res.selected ? 1.0f : 0.0f, SpringFast(), res.selected ? 1.0f : 0.0f);
        const Rect pill = rr.Expanded(-Sc(3), -Sc(1.5f));
        if (selT > 0.01f)
            p.Rect(pill, Style().Radius(Sc(8)).Fill(Accent().Fade(0.24f * selT)));
        if (hover > 0.01f && selT < 0.99f)
            p.Rect(pill, Style().Radius(Sc(8)).Fill(pc.highlight.Fade(hover * 0.7f)));
        if (t.options.flags & TableFlags_ColumnLines)
            for (std::size_t i = 1; i + 1 < t.x.size(); ++i)
                p.FillRect(Rect(t.x[i] - 0.5f, rr.min.y, t.x[i] + 0.5f, rr.max.y), pc.separator.Fade(0.6f));
        return res;
    }

    void TableNextColumn()
    {
        TableFrame& t = CurrentTable();
        Context& c = Ctx();
        CloseCell(t);
        if (t.row < 0 || t.column + 1 >= (int)t.columns.size())
            return;
        ++t.column;
        const Rect cell = CellRect(t, t.column);
        c.PushClipRect(cell);
        c.SetCursorPos(cell.min);
        ContainerOptions co;
        co.size = cell.Size();
        co.padding = Vec2(Sc(10), std::max(0.0f, (t.rowH - Sc(30)) * 0.5f));
        c.BeginContainer(Salt(t.id, 0x20000 + (std::uint32_t)t.column), co);
        t.cellOpen = true;
    }

    namespace
    {
        void CellText(Icon icon, std::string_view text, Color color, Color iconColor)
        {
            TableFrame& t = CurrentTable();
            CloseCell(t);
            if (t.row < 0 || t.column + 1 >= (int)t.columns.size())
                return;
            ++t.column;
            const Rect cell = CellRect(t, t.column);
            const Palette& pc = C();
            Painter p = GetPainter();
            const float padX = Sc(10);
            float x0 = cell.min.x + padX;
            if (icon)
            {
                DrawIcon(p, Vec2(x0 + Sc(8), cell.Center().y), icon, Sc(14), iconColor.a > 0 ? iconColor : pc.secondaryLabel);
                x0 += Sc(24);
            }
            const text::FontRef f = Font(TextStyle::Subheadline);
            const Vec2 ts = MeasureText(f, text);
            const Align al = t.columns[(std::size_t)t.column].align;
            const float ax = al == Align::End ? 1.0f : al == Align::Center ? 0.5f : 0.0f;
            const float y = std::floor(cell.Center().y - ts.y * 0.5f + 0.5f);
            if (cell.max.x - padX > x0)
                p.TextBox(Rect(x0, y, cell.max.x - padX, y + ts.y), Vec2(ax, 0.0f), f, color.a > 0 ? color : LabelOr(pc.label), text, text::TextFlags_Ellipsis);
        }
    }

    void TableCell(std::string_view text, Color color) { CellText(0, text, color, Color::Clear()); }
    void TableCellIcon(Icon icon, std::string_view text, Color iconColor) { CellText(icon, text, Color::Clear(), iconColor); }

    void EndTable()
    {
        TableFrame& t = CurrentTable();
        Context& c = Ctx();
        CloseCell(t);
        const int rows = std::max(t.rowCount, t.rowsSeen);
        const double rowsH = (double)rows * t.rowH;
        if (t.scrolls)
        {
            // the scroll range covers every row, submitted or not
            c.SetCursorPos(Vec2(t.outer.min.x, (float)((double)t.bodyTop - t.scroll + rowsH)));
            c.ItemSize(Vec2(t.outer.Width(), 0.0f));
            EndScrollEdgeFade(t.edgeFade);
            ScrollEnd(t.child, -1.0f);
            c.EndChild();
        }
        if (t.options.flags & TableFlags_ColumnLines)
        {
            Painter p = GetPainter();
            for (std::size_t i = 1; i + 1 < t.x.size(); ++i)
                p.FillRect(Rect(t.x[i] - 0.5f, t.outer.min.y + Sc(6), t.x[i] + 0.5f, t.outer.min.y + t.headerH - Sc(6)), C().separator);
        }
        // the table's height: fixed, or header + every row (this frame's count: the next frame lays it out so)
        c.SetCursorPos(t.outer.min);
        const float height = t.scrolls ? t.outer.Height() : t.headerH + (float)rowsH;
        c.ItemSize(Vec2(t.outer.Width(), height));
        c.EndContainer();
        const bool pushed = t.stylePushed;
        g_tables.pop_back();
        if (pushed)
            PopStyle();
    }

    // ================================================================= trees
    void SetNextTreeNodeOpen(bool open) { g_nextTreeOpen = open; }

    TreeNodeResult TreeNode(std::string_view label, const TreeNodeOptions& o)
    {
        ItemScope scope;
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Theme& th = T();
        const Palette& pc = th.colors;
        TreeNodeResult res;
        const Id id = c.GetId(label);
        TreeState& st = c.State<TreeState>(id);
        const bool leaf = (o.flags & TreeFlags_Leaf) != 0;
        if (!st.known)
        {
            st.open = (o.flags & TreeFlags_DefaultOpen) != 0;
            st.known = true;
        }
        if (g_nextTreeOpen)
        {
            if (!leaf)
                st.open = *g_nextTreeOpen;
            g_nextTreeOpen.reset();
        }

        const float rowH = Sc(30);
        const float indentStep = Sc(18);
        const float depth = (float)g_trees.size();
        const float width = AvailableWidth();
        const Vec2 pos = c.CursorPos();
        c.ItemSize(Vec2(width, rowH));
        const Rect row = Rect::FromSize(pos, Vec2(width, rowH));
        const float x0 = row.min.x + depth * indentStep;
        const Rect arrow = Rect::FromSize(Vec2(x0, row.min.y), Vec2(Sc(22), rowH));

        // the row (selects; toggles unless OpenOnArrow), then the arrow over it (always toggles)
        const ButtonResult b = c.ItemAdd(id, row) ? c.ButtonBehavior(id, row, ButtonFlags_None) : ButtonResult{};
        bool toggle = false;
        Interaction ai;
        if (!leaf)
        {
            ai = InteractImpl(Salt(id, 0xA77), arrow, InteractFlags_None);
            toggle = ai.pressed;
        }
        res.clicked = b.pressed;
        res.doubleClicked = b.pressed && b.clicks >= 2;
        if (!leaf && b.pressed && !(o.flags & TreeFlags_OpenOnArrow))
            toggle = true;
        if (!leaf && (o.flags & TreeFlags_OpenOnArrow) && res.doubleClicked)
            toggle = true;
        if (toggle)
        {
            st.open = !st.open;
            res.toggled = true;
        }

        Painter p = GetPainter();
        const float hover = Anim(id, 0xA1, b.hovered ? 1.0f : 0.0f, SpringFast());
        const bool selected = (o.flags & TreeFlags_Selected) != 0;
        const float selT = Anim(id, 0x5E1, selected ? 1.0f : 0.0f, SpringFast(), selected ? 1.0f : 0.0f);
        const Rect pill(x0 - Sc(4), row.min.y + Sc(1), row.max.x, row.max.y - Sc(1));
        if (selT > 0.01f)
            p.Rect(pill, Style().Radius(Sc(8)).Fill(Accent().Fade(0.24f * selT)));
        if (hover > 0.01f && selT < 0.99f)
            p.Rect(pill, Style().Radius(Sc(8)).Fill(pc.highlight.Fade(hover * 0.7f)));

        const float shown = leaf ? 0.0f : Anim(id, 0x0E7, st.open ? 1.0f : 0.0f, SpringStd(), st.open ? 1.0f : 0.0f);
        if (!leaf)
            Chevron(p, arrow.Center(), Sc(11) * (1.0f + 0.15f * ai.hover), kPi * 0.5f * shown, pc.secondaryLabel.Fade(1.0f + 0.3f * ai.hover));
        float tx = arrow.max.x + Sc(2);
        if (o.icon)
        {
            DrawIcon(p, Vec2(tx + Sc(9), row.Center().y), o.icon, Sc(15), o.iconColor.a > 0 ? o.iconColor : (selected ? Accent() : pc.secondaryLabel));
            tx += Sc(26);
        }
        const text::FontRef f = Font(TextStyle::Subheadline);
        float right = row.max.x - Sc(10);
        if (!o.detail.empty())
        {
            const text::FontRef df = Font(TextStyle::Footnote);
            const Vec2 ds = MeasureText(df, o.detail);
            const float dx = std::max(tx + Sc(40), right - ds.x);
            p.TextBox(Rect(dx, std::floor(row.Center().y - ds.y * 0.5f + 0.5f), right, std::floor(row.Center().y - ds.y * 0.5f + 0.5f) + ds.y), Vec2(1, 0), df,
                      pc.tertiaryLabel, o.detail, text::TextFlags_Ellipsis);
            right = dx - Sc(8);
        }
        const std::string_view shownLabel = VisibleLabel(label);
        const Vec2 ls = MeasureText(f, shownLabel);
        if (right > tx)
            p.TextBox(Rect(tx, std::floor(row.Center().y - ls.y * 0.5f + 0.5f), right, std::floor(row.Center().y - ls.y * 0.5f + 0.5f) + ls.y), Vec2(0, 0), f,
                      LabelOr(pc.label), shownLabel, text::TextFlags_Ellipsis);

        // the children: while open or sliding closed, inside a clip as tall as they are shown
        res.open = !leaf && shown > 0.002f;
        if (res.open)
        {
            TreeFrame f2;
            f2.id = id;
            f2.top = row.max.y;
            f2.x = arrow.Center().x;
            f2.shown = shown;
            f2.clipped = shown < 0.999f;   // while sliding (fully open: nothing to hide, and the first frame has no height yet)
            if (f2.clipped)
                c.PushClipRect(Rect(row.min.x - Sc(8), row.max.y, row.max.x + Sc(8), row.max.y + std::max(st.childHeight, 0.0f) * shown));
            PushStyle(ItemStyle().Opacity(Saturate(shown * 1.4f - 0.2f)));
            c.PushId(label);
            g_trees.push_back(f2);
        }
        return res;
    }

    void TreePop()
    {
        ESIA_ASSERT(!g_trees.empty() && "TreePop without an open TreeNode");
        if (g_trees.empty())
            return;
        const TreeFrame f = g_trees.back();
        g_trees.pop_back();
        Context& c = Ctx();
        c.PopId();
        PopStyle();
        TreeState& st = c.State<TreeState>(f.id);
        const Vec2 cur = c.CursorPos();
        st.childHeight = std::max(cur.y - f.top, 0.0f);
        // the guide line beside the children
        {
            Painter p = GetPainter();
            const float h = st.childHeight * f.shown;
            if (h > Sc(8))
                p.FillRect(Rect(f.x - 0.5f, f.top + Sc(2), f.x + 0.5f, f.top + h - Sc(6)), C().separator.Fade(0.8f));
        }
        if (f.clipped)
            c.PopClipRect();
        if (f.shown < 0.999f)
        {
            // what is shown of them takes the room; the rest is given back to what follows
            c.SetCursorPos(Vec2(cur.x, f.top + st.childHeight * f.shown));
            M().animating = true;
        }
    }
}
