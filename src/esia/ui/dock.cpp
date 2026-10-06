// Esia UI - docking: dock spaces split into nodes; each leaf node holds windows in tabs. A docked window fills its node
// (its header becomes the node's tab bar) behind the floating windows; its tab dragged out floats it again, and a
// floating window dragged over a dock space shows where it would dock (a compass in the node under the mouse, and
// at the space's edges) and docks there when let go.
//
// The layout (the node tree and which window is in which node) lives in the Context's per-id state, keyed by the dock
// space's name, and in a registry of the windows BeginWindow has seen; SaveDockLayout / LoadDockLayout turn it into
// text and back.
#include "ui_internal.hpp"
#include "esia/base/hash.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <unordered_map>

namespace esia::ui
{
    using namespace detail;

    namespace
    {
        constexpr Id kRegistryId = 0xD0C4E51Au;

        struct DockNode
        {
            int parent = -1;
            int child[2] = {-1, -1};        // a split has both
            bool vertical = false;          // the children above one another (else side by side)
            float ratio = 0.5f;             // the first child's share
            std::vector<std::string> windows;   // a leaf's tabs (BeginWindow titles)
            int active = 0;                 // the tab shown
            bool used = true;
            Rect rect;                      // this frame
        };

        struct DockTarget
        {
            int node = -1;                  // -1: the space's root (an edge of the whole space)
            DockSide side = DockSide::Center;
            bool valid = false;
            Rect preview;
        };

        struct DockSpaceData
        {
            std::vector<DockNode> nodes;    // [0] = the root
            Rect rect;
            std::uint64_t frame = 0;        // the last frame DockSpace was called
            std::string moving;             // a floating window being dragged over it (last frame)
            DockTarget target;
            bool init = false;
        };

        struct DockedWindow
        {
            std::string space;              // empty: floating
            Icon icon = 0;
            bool closable = false;
            bool closeRequested = false;    // its tab's close button
            std::uint64_t lastFrame = 0;    // the last frame BeginWindow was called for it
            bool pendingMove = false;       // just undocked by a tab drag: float under the mouse and keep moving
            Vec2 grab;
        };

        struct Registry
        {
            std::unordered_map<std::string, DockedWindow> windows;
            struct TabDrag
            {
                std::string window, space;
                Vec2 start;
                bool active = false;
            } tabDrag;
        };

        Context& Cx() { return Ctx(); }
        Registry& Reg() { return Cx().State<Registry>(kRegistryId); }
        Id SpaceId(std::string_view name) { return HashLabel(name, 0x5D0C5ACEu); }

        DockSpaceData& Space(std::string_view name)
        {
            DockSpaceData& d = Cx().State<DockSpaceData>(SpaceId(name));
            if (!d.init)
            {
                d.nodes.assign(1, DockNode{});
                d.init = true;
            }
            return d;
        }

        bool IsLeaf(const DockNode& n) { return n.child[0] < 0; }

        int NewNode(DockSpaceData& d)
        {
            for (std::size_t i = 1; i < d.nodes.size(); ++i)
                if (!d.nodes[i].used)
                {
                    d.nodes[i] = DockNode{};
                    return (int)i;
                }
            d.nodes.push_back(DockNode{});
            return (int)d.nodes.size() - 1;
        }

        int FindLeaf(const DockSpaceData& d, std::string_view window)
        {
            for (std::size_t i = 0; i < d.nodes.size(); ++i)
                if (d.nodes[i].used && IsLeaf(d.nodes[i]) && std::find(d.nodes[i].windows.begin(), d.nodes[i].windows.end(), window) != d.nodes[i].windows.end())
                    return (int)i;
            return -1;
        }

        int FirstLeaf(const DockSpaceData& d, int i)
        {
            while (!IsLeaf(d.nodes[(std::size_t)i]))
                i = d.nodes[(std::size_t)i].child[0];
            return i;
        }

        void SetParent(DockSpaceData& d, int node, int parent)
        {
            d.nodes[(std::size_t)node].parent = parent;
        }

        // An empty leaf goes: its sibling takes the parent's place.
        void Collapse(DockSpaceData& d, int leaf)
        {
            if (leaf <= 0)
                return;   // the root stays (empty)
            const int p = d.nodes[(std::size_t)leaf].parent;
            const DockNode& pn = d.nodes[(std::size_t)p];
            const int sib = pn.child[0] == leaf ? pn.child[1] : pn.child[0];
            DockNode moved = d.nodes[(std::size_t)sib];
            moved.parent = pn.parent;
            d.nodes[(std::size_t)p] = moved;
            if (!IsLeaf(moved))
            {
                SetParent(d, moved.child[0], p);
                SetParent(d, moved.child[1], p);
            }
            d.nodes[(std::size_t)leaf].used = false;
            d.nodes[(std::size_t)sib].used = false;
        }

        void Remove(DockSpaceData& d, std::string_view window)
        {
            const int leaf = FindLeaf(d, window);
            if (leaf < 0)
                return;
            DockNode& n = d.nodes[(std::size_t)leaf];
            const auto it = std::find(n.windows.begin(), n.windows.end(), window);
            const int index = (int)(it - n.windows.begin());
            n.windows.erase(it);
            if (n.active > index || n.active >= (int)n.windows.size())
                n.active = std::max(0, n.active - 1);
            if (n.windows.empty())
                Collapse(d, leaf);
        }

        void Insert(DockSpaceData& d, const std::string& window, DockSide side, float ratio, int target)
        {
            if (target < 0 || target >= (int)d.nodes.size() || !d.nodes[(std::size_t)target].used)
                target = 0;
            const bool emptyLeaf = IsLeaf(d.nodes[(std::size_t)target]) && d.nodes[(std::size_t)target].windows.empty();
            if (side == DockSide::Center || emptyLeaf)
            {
                const int leaf = FirstLeaf(d, target);
                DockNode& n = d.nodes[(std::size_t)leaf];
                n.windows.push_back(window);
                n.active = (int)n.windows.size() - 1;
                return;
            }
            // split: the target's content moves into a new node beside a new leaf with the window
            const int a = NewNode(d);
            const int b = NewNode(d);
            d.nodes[(std::size_t)a] = d.nodes[(std::size_t)target];
            d.nodes[(std::size_t)a].parent = target;
            if (!IsLeaf(d.nodes[(std::size_t)a]))
            {
                SetParent(d, d.nodes[(std::size_t)a].child[0], a);
                SetParent(d, d.nodes[(std::size_t)a].child[1], a);
            }
            d.nodes[(std::size_t)b] = DockNode{};
            d.nodes[(std::size_t)b].parent = target;
            d.nodes[(std::size_t)b].windows.push_back(window);
            DockNode& t = d.nodes[(std::size_t)target];
            t.windows.clear();
            t.active = 0;
            t.vertical = side == DockSide::Top || side == DockSide::Bottom;
            const bool before = side == DockSide::Left || side == DockSide::Top;
            t.child[0] = before ? b : a;
            t.child[1] = before ? a : b;
            ratio = std::clamp(ratio, 0.08f, 0.92f);
            t.ratio = before ? ratio : 1.0f - ratio;
        }

        void LayoutNodes(DockSpaceData& d, int i, Rect r, float gap)
        {
            DockNode& n = d.nodes[(std::size_t)i];
            n.rect = r;
            if (IsLeaf(n))
                return;
            const int c0 = n.child[0], c1 = n.child[1];
            if (!n.vertical)
            {
                const float w = std::max(0.0f, r.Width() - gap) * n.ratio;
                LayoutNodes(d, c0, Rect(r.min.x, r.min.y, r.min.x + w, r.max.y), gap);
                LayoutNodes(d, c1, Rect(r.min.x + w + gap, r.min.y, r.max.x, r.max.y), gap);
            }
            else
            {
                const float h = std::max(0.0f, r.Height() - gap) * n.ratio;
                LayoutNodes(d, c0, Rect(r.min.x, r.min.y, r.max.x, r.min.y + h), gap);
                LayoutNodes(d, c1, Rect(r.min.x, r.min.y + h + gap, r.max.x, r.max.y), gap);
            }
        }

        // A tab is shown while its window is still submitted (BeginWindow in the last two frames, or this one): the same
        // answer for every window of a node, whichever of them BeginWindow sees first this frame.
        bool Visible(const Registry& reg, const std::string& window, std::uint64_t frame)
        {
            const auto it = reg.windows.find(window);
            return it != reg.windows.end() && it->second.lastFrame + 2 >= frame;
        }

        // The tab shown: the node's active one, else (it is not submitted) the first one that is. The node keeps its
        // choice for when that window comes back.
        int ActiveVisible(const Registry& reg, const DockNode& n, std::uint64_t frame)
        {
            if (n.active >= 0 && n.active < (int)n.windows.size() && Visible(reg, n.windows[(std::size_t)n.active], frame))
                return n.active;
            for (int i = 0; i < (int)n.windows.size(); ++i)
                if (Visible(reg, n.windows[(std::size_t)i], frame))
                    return i;
            return -1;
        }

        Painter OverlayPainter()
        {
            Ui::Impl& m = M();
            PainterEnv env;
            env.metricsScale = T().metrics.scale;
            env.cornerSmoothing = T().metrics.cornerSmoothing;
            env.pixelScale = m.ctx->Scale();
            env.text = m.text;
            env.flat = m.flat;
            env.flatSurface = T().colors.secondaryBackground;
            return Painter(m.ctx->ForegroundDrawList(), env);
        }

        // The compass in a node, or the targets at the space's edges, under the mouse.
        DockTarget HitTarget(const DockSpaceData& d, Vec2 mouse, float targetSize, std::vector<std::pair<Rect, DockTarget>>* all)
        {
            DockTarget hit;
            const auto add = [&](const Rect& r, int node, DockSide side, const Rect& preview) {
                DockTarget t;
                t.node = node;
                t.side = side;
                t.valid = true;
                t.preview = preview;
                if (all)
                    all->push_back({r, t});
                if (r.Contains(mouse))
                    hit = t;
            };
            const auto half = [](const Rect& r, DockSide side, float ratio) {
                switch (side)
                {
                case DockSide::Left: return Rect(r.min.x, r.min.y, r.min.x + r.Width() * ratio, r.max.y);
                case DockSide::Right: return Rect(r.max.x - r.Width() * ratio, r.min.y, r.max.x, r.max.y);
                case DockSide::Top: return Rect(r.min.x, r.min.y, r.max.x, r.min.y + r.Height() * ratio);
                case DockSide::Bottom: return Rect(r.min.x, r.max.y - r.Height() * ratio, r.max.x, r.max.y);
                default: return r;
                }
            };
            if (!d.rect.Contains(mouse))
                return hit;
            // the leaf under the mouse: its compass
            for (std::size_t i = 0; i < d.nodes.size(); ++i)
            {
                const DockNode& n = d.nodes[i];
                if (!n.used || !IsLeaf(n) || !n.rect.Contains(mouse))
                    continue;
                const Vec2 cc = n.rect.Center();
                const float s = targetSize, g = targetSize + Sc(6);
                add(Rect::FromCenter(cc, Vec2(s, s)), (int)i, DockSide::Center, n.rect);
                if (!n.windows.empty())
                {
                    add(Rect::FromCenter(cc - Vec2(g, 0), Vec2(s, s)), (int)i, DockSide::Left, half(n.rect, DockSide::Left, 0.5f));
                    add(Rect::FromCenter(cc + Vec2(g, 0), Vec2(s, s)), (int)i, DockSide::Right, half(n.rect, DockSide::Right, 0.5f));
                    add(Rect::FromCenter(cc - Vec2(0, g), Vec2(s, s)), (int)i, DockSide::Top, half(n.rect, DockSide::Top, 0.5f));
                    add(Rect::FromCenter(cc + Vec2(0, g), Vec2(s, s)), (int)i, DockSide::Bottom, half(n.rect, DockSide::Bottom, 0.5f));
                }
            }
            // the space's edges: beside everything
            const bool split = !IsLeaf(d.nodes[0]);
            if (split)
            {
                const Rect& r = d.rect;
                const float s = targetSize, in = targetSize * 0.5f + Sc(10);
                add(Rect::FromCenter(Vec2(r.min.x + in, r.Center().y), Vec2(s, s)), -1, DockSide::Left, half(r, DockSide::Left, 0.3f));
                add(Rect::FromCenter(Vec2(r.max.x - in, r.Center().y), Vec2(s, s)), -1, DockSide::Right, half(r, DockSide::Right, 0.3f));
                add(Rect::FromCenter(Vec2(r.Center().x, r.min.y + in), Vec2(s, s)), -1, DockSide::Top, half(r, DockSide::Top, 0.3f));
                add(Rect::FromCenter(Vec2(r.Center().x, r.max.y - in), Vec2(s, s)), -1, DockSide::Bottom, half(r, DockSide::Bottom, 0.3f));
            }
            return hit;
        }

        Icon SideIcon(DockSide side)
        {
            switch (side)
            {
            case DockSide::Left: return icons::ChevronLeft;
            case DockSide::Right: return icons::ChevronRight;
            case DockSide::Top: return icons::ChevronUp;
            case DockSide::Bottom: return icons::ChevronDown;
            default: return icons::Apps;
            }
        }
    }

    // ============================================================= dock space
    void DockSpace(std::string_view id, Rect rect)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Palette& pc = C();
        const InputState& in = c.Input();
        if (rect.Width() <= 0.0f || rect.Height() <= 0.0f)
            rect = c.SafeArea();
        DockSpaceData& d = Space(id);
        Registry& reg = Reg();
        d.frame = c.FrameCount();
        d.rect = rect;
        const float gap = Sc(8);
        LayoutNodes(d, 0, rect, gap);

        // the host: splitters and empty nodes, behind every window
        c.SetNextWindowPos(rect.min, Cond::Always);
        c.SetNextWindowSize(rect.Size(), Cond::Always);
        esia::WindowOptions wo;
        wo.flags = esia::WindowFlags_NoMove | esia::WindowFlags_NoResize | esia::WindowFlags_NoBringToFront | esia::WindowFlags_NoFocus |
                   esia::WindowFlags_NoScroll;
        wo.layer = WindowLayer::Background;
        wo.padding = Vec2(0, 0);
        wo.minSize = Vec2(1, 1);
        const std::string host = "##dockspace/" + std::string(id);
        if (c.Begin(host, wo))
        {
            Painter p = GetPainter();
            const Id sid = SpaceId(id);
            for (std::size_t i = 0; i < d.nodes.size(); ++i)
            {
                DockNode& n = d.nodes[i];
                if (!n.used)
                    continue;
                if (IsLeaf(n))
                {
                    if (ActiveVisible(reg, n, d.frame) < 0)
                        p.Rect(n.rect.Expanded(-Sc(2)), Style().Radius(Sc(16)).Fill(pc.fill.Fade(0.18f)).Stroke(1.0f, pc.separator.Fade(0.5f)));
                    continue;
                }
                // the splitter in the gap between the children
                const Rect& a = d.nodes[(std::size_t)n.child[0]].rect;
                const Rect sr = n.vertical ? Rect(n.rect.min.x, a.max.y, n.rect.max.x, a.max.y + gap) : Rect(a.max.x, n.rect.min.y, a.max.x + gap, n.rect.max.y);
                const Interaction it = InteractImpl(Salt(sid, 0x5000 + (std::uint32_t)i), sr, InteractFlags_PressOnClick);
                if (it.held && in.MouseValid())
                {
                    const float t = n.vertical ? (in.MousePos().y - gap * 0.5f - n.rect.min.y) / std::max(n.rect.Height() - gap, 1.0f)
                                               : (in.MousePos().x - gap * 0.5f - n.rect.min.x) / std::max(n.rect.Width() - gap, 1.0f);
                    n.ratio = std::clamp(t, 0.08f, 0.92f);
                }
                if (it.hovered || it.held)
                    c.SetMouseCursor(n.vertical ? MouseCursor::ResizeNS : MouseCursor::ResizeEW);
                const float show = std::max(it.hover, it.press);
                if (show > 0.01f)
                {
                    const Vec2 cc = sr.Center();
                    const Rect handle = n.vertical ? Rect::FromCenter(cc, Vec2(Sc(44), Sc(5))) : Rect::FromCenter(cc, Vec2(Sc(5), Sc(44)));
                    p.Capsule(handle, Style().Fill((it.held ? Accent() : pc.label).Fade(0.25f + 0.45f * show)));
                }
            }
            if (!IsLeaf(d.nodes[0]))
                LayoutNodes(d, 0, rect, gap);   // a splitter moved: this frame's windows take the new rects

            // ---- a floating window dragged over the space: where it would dock
            const Window* moving = c.MovingWindow();
            const bool dockable = moving && reg.windows.count(moving->Name()) && FindLeaf(d, moving->Name()) < 0;
            if (dockable)
            {
                d.moving = moving->Name();
                std::vector<std::pair<Rect, DockTarget>> all;
                d.target = HitTarget(d, in.MousePos(), Sc(38), &all);
                if (d.rect.Contains(in.MousePos()))
                {
                    Painter fp = OverlayPainter();
                    if (d.target.valid)
                        fp.Rect(d.target.preview.Expanded(-Sc(3)), Style().Radius(Sc(16)).Fill(Accent().Fade(0.16f)).Stroke(Sc(2), Accent().Fade(0.7f)));
                    for (const auto& [r, t] : all)
                    {
                        const bool hot = d.target.valid && t.node == d.target.node && t.side == d.target.side;
                        fp.Rect(r, Style().Radius(Sc(10)).Glass(LookMaterial(T().materials.control)).Fill((hot ? Accent() : pc.windowSurface).Fade(hot ? 0.85f : 0.7f))
                                       .Shadow(pc.shadow, Sc(10), Vec2(0, Sc(3))));
                        DrawIcon(fp, r.Center(), SideIcon(t.side), Sc(15), hot ? Color::White() : pc.label);
                    }
                }
            }
            else if (!d.moving.empty())
            {
                // the drag ended: let go over a target, the window docks there
                if (in.MouseReleased(MouseButton::Left) && d.target.valid)
                {
                    const DockTarget t = d.target;
                    if (t.node < 0)
                    {
                        // an edge of the whole space: beside everything in it
                        UndockWindow(d.moving);
                        Insert(d, d.moving, t.side, 0.3f, 0);
                        reg.windows[d.moving].space = std::string(id);
                    }
                    else
                    {
                        const DockNode& n = d.nodes[(std::size_t)t.node];
                        DockWindow(d.moving, id, t.side, 0.5f, n.windows.empty() ? std::string_view() : std::string_view(n.windows.front()));
                    }
                }
                d.moving.clear();
                d.target = DockTarget{};
            }
        }
        c.End();
    }

    // ============================================================ the layout
    void DockWindow(std::string_view window, std::string_view dockSpace, DockSide side, float ratio, std::string_view relativeTo)
    {
        Registry& reg = Reg();
        const std::string name(window);
        DockedWindow& e = reg.windows[name];
        if (!e.space.empty())
            Remove(Space(e.space), name);
        DockSpaceData& d = Space(dockSpace);
        Remove(d, name);
        const int target = relativeTo.empty() ? 0 : FindLeaf(d, relativeTo);
        Insert(d, name, side, ratio, target);
        e.space = std::string(dockSpace);
        e.pendingMove = false;
    }

    void UndockWindow(std::string_view window)
    {
        Registry& reg = Reg();
        const auto it = reg.windows.find(std::string(window));
        if (it == reg.windows.end() || it->second.space.empty())
            return;
        Remove(Space(it->second.space), window);
        it->second.space.clear();
    }

    void FocusDockedWindow(std::string_view window)
    {
        Registry& reg = Reg();
        const auto it = reg.windows.find(std::string(window));
        if (it == reg.windows.end() || it->second.space.empty())
            return;
        DockSpaceData& d = Space(it->second.space);
        const int leaf = FindLeaf(d, window);
        if (leaf < 0)
            return;
        DockNode& n = d.nodes[(std::size_t)leaf];
        n.active = (int)(std::find(n.windows.begin(), n.windows.end(), window) - n.windows.begin());
    }

    bool IsWindowDocked(std::string_view window)
    {
        Registry& reg = Reg();
        const auto it = reg.windows.find(std::string(window));
        return it != reg.windows.end() && !it->second.space.empty() && FindLeaf(Space(it->second.space), window) >= 0;
    }

    // "esia-dock 1", then a line per node: index, parent, "split" vertical ratio child0 child1 | "leaf" active and the
    // tab titles separated by tabs
    std::string SaveDockLayout(std::string_view dockSpace)
    {
        const DockSpaceData& d = Space(dockSpace);
        std::string out = "esia-dock 1\n";
        char buf[160];
        for (std::size_t i = 0; i < d.nodes.size(); ++i)
        {
            const DockNode& n = d.nodes[i];
            if (!n.used)
                continue;
            if (!IsLeaf(n))
            {
                std::snprintf(buf, sizeof(buf), "%zu %d split %d %.4f %d %d\n", i, n.parent, n.vertical ? 1 : 0, (double)n.ratio, n.child[0], n.child[1]);
                out += buf;
            }
            else
            {
                std::snprintf(buf, sizeof(buf), "%zu %d leaf %d", i, n.parent, n.active);
                out += buf;
                for (const std::string& w : n.windows)
                    out += '\t' + w;
                out += '\n';
            }
        }
        return out;
    }

    bool LoadDockLayout(std::string_view dockSpace, std::string_view layout)
    {
        std::vector<DockNode> nodes;
        std::vector<char> seen;
        std::size_t pos = 0;
        const auto nextLine = [&](std::string& line) {
            if (pos >= layout.size())
                return false;
            std::size_t e = layout.find('\n', pos);
            if (e == std::string_view::npos)
                e = layout.size();
            line.assign(layout.substr(pos, e - pos));
            pos = e + 1;
            return true;
        };
        std::string line;
        if (!nextLine(line) || line != "esia-dock 1")
            return false;
        while (nextLine(line))
        {
            if (line.empty())
                continue;
            const std::size_t tab = line.find('\t');
            std::istringstream ss(line.substr(0, tab));
            std::size_t index = 0;
            int parent = -1;
            std::string kind;
            if (!(ss >> index >> parent >> kind) || index > 4096)
                return false;
            if (nodes.size() <= index)
            {
                nodes.resize(index + 1);
                seen.resize(index + 1, 0);
            }
            seen[index] = 1;
            DockNode n;
            n.parent = parent;
            if (kind == "split")
            {
                int v = 0;
                if (!(ss >> v >> n.ratio >> n.child[0] >> n.child[1]))
                    return false;
                n.vertical = v != 0;
            }
            else if (kind == "leaf")
            {
                ss >> n.active;
                for (std::size_t t = tab; t != std::string::npos;)
                {
                    const std::size_t e = line.find('\t', t + 1);
                    n.windows.push_back(line.substr(t + 1, e == std::string::npos ? std::string::npos : e - t - 1));
                    t = e;
                }
            }
            else
                return false;
            nodes[index] = std::move(n);
        }
        for (std::size_t i = 0; i < nodes.size(); ++i)
            nodes[i].used = seen[i] != 0;
        if (nodes.empty() || !nodes[0].used)
            return false;
        for (const DockNode& n : nodes)
            if (n.used && !IsLeaf(n) && (n.child[0] < 0 || n.child[1] < 0 || n.child[0] >= (int)nodes.size() || n.child[1] >= (int)nodes.size()))
                return false;
        // the windows of the old layout float; those of the new one dock here
        Registry& reg = Reg();
        DockSpaceData& d = Space(dockSpace);
        for (auto& [name, e] : reg.windows)
            if (e.space == dockSpace)
                e.space.clear();
        d.nodes = std::move(nodes);
        for (const DockNode& n : d.nodes)
            if (n.used)
                for (const std::string& w : n.windows)
                {
                    DockedWindow& e = reg.windows[w];
                    if (!e.space.empty() && e.space != dockSpace)
                        Remove(Space(e.space), w);
                    e.space = std::string(dockSpace);
                }
        return true;
    }

    // ===================================================== for BeginWindow
    bool detail::DockedPlacement(std::string_view title, bool closable, Icon icon, DockPlacement& out)
    {
        Context& c = Cx();
        Registry& reg = Reg();
        DockedWindow& e = reg.windows[std::string(title)];
        e.icon = icon;
        e.closable = closable;
        e.lastFrame = c.FrameCount();
        out = DockPlacement{};
        if (e.pendingMove)
        {
            // undocked by its tab this frame or the last: floats under the mouse and goes on moving with it
            out.pendingMove = true;
            out.grab = e.grab;
            e.pendingMove = false;
            return false;
        }
        if (e.space.empty())
            return false;
        DockSpaceData& d = Space(e.space);
        if (d.frame != c.FrameCount())
            return false;   // its dock space is not shown this frame: it floats
        const int leaf = FindLeaf(d, title);
        if (leaf < 0)
        {
            e.space.clear();
            return false;
        }
        DockNode& n = d.nodes[(std::size_t)leaf];
        const int active = ActiveVisible(reg, n, c.FrameCount());
        out.rect = n.rect;
        out.shown = active >= 0 && n.windows[(std::size_t)active] == title;
        out.closeRequested = e.closeRequested;
        e.closeRequested = false;
        return true;
    }

    float detail::DockTabBar(std::string_view title, const Rect& header)
    {
        Ui::Impl& m = M();
        Context& c = *m.ctx;
        const Palette& pc = C();
        const InputState& in = c.Input();
        Registry& reg = Reg();
        const auto self = reg.windows.find(std::string(title));
        if (self == reg.windows.end() || self->second.space.empty())
            return 0.0f;
        const std::string spaceName = self->second.space;
        DockSpaceData& d = Space(spaceName);
        const int leaf = FindLeaf(d, title);
        if (leaf < 0)
            return 0.0f;
        const Id sid = SpaceId(spaceName);
        std::vector<int> tabs;
        for (int i = 0; i < (int)d.nodes[(std::size_t)leaf].windows.size(); ++i)
            if (Visible(reg, d.nodes[(std::size_t)leaf].windows[(std::size_t)i], c.FrameCount()))
                tabs.push_back(i);
        Painter p = GetPainter();
        const float pad = Sc(8);
        const float h = header.Height() - pad * 1.25f;
        const float avail = header.Width() - pad * 2.0f;
        const float tw = tabs.empty() ? 0.0f : std::clamp((avail - Sc(4) * (float)(tabs.size() - 1)) / (float)tabs.size(), Sc(48), Sc(190));
        const text::FontRef f = Font(TextStyle::Subheadline);
        const text::FontRef fa = Font(FontWeight::Semibold, T().type.size[(int)TextStyle::Subheadline]);
        float x = header.min.x + pad;
        for (const int i : tabs)
        {
            DockNode& n = d.nodes[(std::size_t)leaf];
            const std::string w = n.windows[(std::size_t)i];
            const DockedWindow& we = reg.windows[w];
            const bool active = w == title;
            const Rect tr = Rect::FromSize(Vec2(x, header.min.y + pad), Vec2(tw, h));
            x += tw + Sc(4);
            const Id tid = Salt(sid, HashLabel(w, 0x7AB5u));
            const Interaction it = InteractImpl(tid, tr, InteractFlags_PressOnClick);
            if (it.pressed)
            {
                n.active = i;
                reg.tabDrag.window = w;
                reg.tabDrag.space = spaceName;
                reg.tabDrag.start = in.MousePos();
                reg.tabDrag.active = true;
            }
            const float sel = Anim(tid, 0x5E1, active ? 1.0f : 0.0f, SpringFast(), active ? 1.0f : 0.0f);
            if (sel > 0.01f)
                p.Rect(tr, Style().Radius(h * 0.5f).Fill(pc.fill.Fade(0.9f * sel)).Shadow(pc.shadow.Fade(0.4f * sel), Sc(6), Vec2(0, Sc(1))));
            if (it.hover > 0.01f && !active)
                p.Rect(tr, Style().Radius(h * 0.5f).Fill(pc.highlight.Fade(it.hover * 0.8f)));
            float lx = tr.min.x + Sc(12);
            if (we.icon)
            {
                DrawIcon(p, Vec2(lx + Sc(7), tr.Center().y), we.icon, Sc(13), active ? Accent() : pc.secondaryLabel);
                lx += Sc(20);
            }
            const bool closeShown = we.closable && (it.hovered || active);
            const float right = tr.max.x - (closeShown ? Sc(26) : Sc(10));
            const std::string_view label = VisibleLabel(w);
            const Vec2 ls = MeasureText(active ? fa : f, label);
            if (right > lx)
                p.TextBox(Rect(lx, std::floor(tr.Center().y - ls.y * 0.5f + 0.5f), right, std::floor(tr.Center().y - ls.y * 0.5f + 0.5f) + ls.y), Vec2(0, 0),
                          active ? fa : f, active ? pc.label : pc.secondaryLabel, label, text::TextFlags_Ellipsis);
            if (closeShown)
            {
                const Vec2 cc(tr.max.x - Sc(14), tr.Center().y);
                const Interaction ci = InteractImpl(Salt(tid, 0xC1), Rect::FromCenter(cc, Vec2(Sc(18), Sc(18))), InteractFlags_None);
                if (ci.hover > 0.01f)
                    p.Circle(cc, Sc(9), Style().Fill(pc.highlight.Fade(ci.hover)));
                DrawIcon(p, cc, icons::Close, Sc(8), pc.secondaryLabel);
                if (ci.pressed)
                    reg.windows[w].closeRequested = true;
            }
        }

        // a tab dragged away from the bar: its window floats and goes on moving with the mouse
        Registry::TabDrag& drag = reg.tabDrag;
        if (drag.active)
        {
            if (!in.MouseDown(MouseButton::Left))
                drag.active = false;
            else if (drag.space == spaceName && std::find(d.nodes[(std::size_t)leaf].windows.begin(), d.nodes[(std::size_t)leaf].windows.end(), drag.window) !=
                                                     d.nodes[(std::size_t)leaf].windows.end())
            {
                const Vec2 delta = in.MousePos() - drag.start;
                if (std::fabs(delta.y) > Sc(18) || std::fabs(delta.x) > Sc(60))
                {
                    DockedWindow& we = reg.windows[drag.window];
                    Remove(d, drag.window);
                    we.space.clear();
                    we.pendingMove = true;
                    we.grab = Vec2(Sc(60), Sc(22));
                    drag.active = false;
                    if (c.ActiveId() != 0)
                        c.ClearActiveId();
                }
            }
        }
        return header.Height();
    }
}
