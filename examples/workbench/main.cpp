// workbench - a tool's layout on Esia's data widgets: a dock space holding an outline (a tree), an inspector (number,
// vector and color fields), a table of 100 000 assets (sorted, filtered, selected), a script editor and a scene view.
// Drag a tab out of its panel to float it, drag a floating panel over the dock space to dock it again; the lines
// between the panels resize them.
//
//   workbench [--float <panel>] [--show <panel>] [--light] [--rows N]       and glass_window's options (../glass_window/app.hpp)
#include "app.hpp"
#include "esia/render/painter.hpp"
#include "esia/ui/ui.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace
{
    using namespace esia;
    namespace ui = esia::ui;
    namespace icon = esia::ui::icons;

    struct Object
    {
        std::string name;
        ui::Icon icon = 0;
        int parent = -1;
        std::vector<int> children;
        float pos[3] = {0, 0, 0};
        float rotation[3] = {0, 0, 0};
        float scale[3] = {1, 1, 1};
        Color color = Color::Hex(0x0A84FF);
        float opacity = 1.0f;
        int layer = 0;
        bool visible = true;
        bool shape = false;   // drawn in the scene view
    };

    struct Asset
    {
        std::string name;
        int type = 0;
        int size = 0;        // KB
        int modified = 0;    // days ago
    };
    constexpr const char* kTypes[] = {"Texture", "Mesh", "Material", "Audio", "Script", "Prefab", "Animation", "Shader"};
    constexpr ui::Icon kTypeIcons[] = {icon::Photo, icon::Apps, icon::Palette, icon::Music, icon::Code, icon::Document, icon::Video, icon::Lightning};

    struct Demo
    {
        Context* ctx = nullptr;
        std::unique_ptr<ui::Ui> ui;
        bool dark = true;
        bool laidOut = false;
        std::string floatPanel, showPanel;
        int frame = 0;

        std::vector<Object> objects;
        int selected = 2;

        std::vector<Asset> assets;
        std::vector<int> view;            // assets shown, in order (filtered, sorted)
        std::vector<int> selection;       // rows of `view`
        std::string filter, shownFilter = "\x01";
        ui::TableSort sort;

        std::string script;
        std::string notes = "Panels dock into this space.\nDrag a tab out to float its panel.";
        bool openScene = true, openOutline = true, openInspector = true, openAssets = true, openScript = true, openNotes = true;

        int Add(std::string name, ui::Icon ic, int parent, bool shape = false)
        {
            Object o;
            o.name = std::move(name);
            o.icon = ic;
            o.parent = parent;
            o.shape = shape;
            objects.push_back(o);
            const int i = (int)objects.size() - 1;
            if (parent >= 0)
                objects[(std::size_t)parent].children.push_back(i);
            return i;
        }

        void Build(int rows)
        {
            const int root = Add("Level 01", icon::World, -1);
            const int env = Add("Environment", icon::Folder, root);
            Add("Sun", icon::Brightness, env);
            Add("Sky", icon::Cloud, env);
            Add("Fog", icon::Cloud, env);
            const int chars = Add("Characters", icon::People, root);
            const Color tints[] = {Color::Hex(0x0A84FF), Color::Hex(0xFF375F), Color::Hex(0x30D158), Color::Hex(0xFF9F0A), Color::Hex(0xBF5AF2)};
            const char* names[] = {"Player", "Guide", "Merchant", "Guard", "Fox"};
            for (int i = 0; i < 5; ++i)
            {
                const int o = Add(names[i], icon::Contact, chars, true);
                objects[(std::size_t)o].color = tints[i];
                objects[(std::size_t)o].pos[0] = -0.6f + 0.3f * (float)i;
                objects[(std::size_t)o].pos[1] = (i % 2) ? 0.25f : -0.2f;
                objects[(std::size_t)o].scale[0] = objects[(std::size_t)o].scale[1] = 0.8f + 0.1f * (float)i;
            }
            const int props = Add("Props", icon::Folder, root);
            const int crates = Add("Crates", icon::Folder, props);
            for (int i = 0; i < 4; ++i)
            {
                char n[32];
                std::snprintf(n, sizeof(n), "Crate %02d", i + 1);
                const int o = Add(n, icon::Apps, crates, true);
                objects[(std::size_t)o].color = Color::Hex(0x8E8E93);
                objects[(std::size_t)o].pos[0] = -0.7f + 0.2f * (float)i;
                objects[(std::size_t)o].pos[1] = 0.65f;
                objects[(std::size_t)o].scale[0] = objects[(std::size_t)o].scale[1] = 0.5f;
            }
            Add("Lamp", icon::Lightbulb, props, true);
            objects.back().color = Color::Hex(0xFFD60A);
            objects.back().pos[0] = 0.7f;
            objects.back().pos[1] = -0.6f;
            Add("Audio Zone", icon::Volume, root);
            selected = chars + 1;

            // assets: deterministic names, sizes and dates
            std::mt19937 rng(7);
            const char* stems[] = {"rock", "grass", "hero", "door", "crate", "sky", "water", "torch", "banner", "bridge", "tree", "stone", "lamp", "cloud", "fence"};
            assets.resize((std::size_t)rows);
            for (int i = 0; i < rows; ++i)
            {
                Asset& a = assets[(std::size_t)i];
                a.type = (int)(rng() % 8);
                char n[64];
                std::snprintf(n, sizeof(n), "%s_%s_%05d", stems[rng() % 15], kTypes[a.type], i);
                for (char* p = n; *p; ++p)
                    *p = (char)std::tolower((unsigned char)*p);
                a.name = n;
                a.size = 4 + (int)(rng() % 40000);
                a.modified = (int)(rng() % 900);
            }

            script =
                "// player.esx - movement and the interaction prompt\n"
                "fn update(dt: f32) {\n"
                "    let input = read_input();\n"
                "    velocity = lerp(velocity, input.move * speed, 1.0 - exp(-12.0 * dt));\n"
                "    position += velocity * dt;\n"
                "\n"
                "    if let Some(target) = nearest_interactable(position, 1.5) {\n"
                "        ui::island(\"Press E to talk to \" + target.name);\n"
                "    }\n"
                "}\n";
        }

        void Refilter()
        {
            if (shownFilter == filter && !sort.changed)
                return;
            view.clear();
            for (int i = 0; i < (int)assets.size(); ++i)
                if (filter.empty() || assets[(std::size_t)i].name.find(filter) != std::string::npos)
                    view.push_back(i);
            if (sort.column >= 0)
            {
                const int col = sort.column;
                const bool asc = sort.ascending;
                std::stable_sort(view.begin(), view.end(), [&](int a, int b) {
                    const Asset& x = assets[(std::size_t)(asc ? a : b)];
                    const Asset& y = assets[(std::size_t)(asc ? b : a)];
                    switch (col)
                    {
                    case 0: return x.name < y.name;
                    case 1: return std::strcmp(kTypes[x.type], kTypes[y.type]) < 0;
                    case 2: return x.size < y.size;
                    default: return x.modified < y.modified;
                    }
                });
            }
            selection.clear();
            shownFilter = filter;
        }

        void Background()
        {
            const Vec2 d = ctx->DisplaySize();
            PainterEnv env;
            env.pixelScale = ctx->FramebufferScale().x;
            Painter p(ctx->BackgroundDrawList(), env);
            const Rect screen(0, 0, d.x, d.y);
            p.Rect(screen, Style().Fill(Paint::Linear(dark ? Color::Hex(0x10132A) : Color::Hex(0xDDE6F5), dark ? Color::Hex(0x2A1033) : Color::Hex(0xF4E3EE), 35)));
            const Color blobs[] = {Color::Hex(0x0A84FF, 0.45f), Color::Hex(0xFF375F, 0.40f), Color::Hex(0x30D158, 0.35f), Color::Hex(0xBF5AF2, 0.40f)};
            const float t = (float)ui::Time();
            for (int i = 0; i < 4; ++i)
            {
                const float a = t * (0.05f + 0.02f * (float)i) + (float)i * 1.7f;
                const Vec2 c(d.x * (0.5f + 0.38f * std::cos(a)), d.y * (0.5f + 0.34f * std::sin(a * 1.3f)));
                const float r = std::min(d.x, d.y) * (0.32f + 0.05f * (float)i);
                p.Circle(c, r, Style().Fill(Paint::Radial(blobs[i], blobs[i].WithAlpha(0.0f))));
            }
        }

        void Outline(int i)
        {
            const Object& o = objects[(std::size_t)i];
            ui::TreeNodeOptions to;
            to.icon = o.icon;
            to.flags = (o.children.empty() ? ui::TreeFlags_Leaf : ui::TreeFlags_None) | (i == selected ? ui::TreeFlags_Selected : 0u) |
                       (o.parent < 0 || objects[(std::size_t)o.parent].parent < 0 ? ui::TreeFlags_DefaultOpen : 0u) | ui::TreeFlags_OpenOnArrow;
            char detail[16] = "";
            if (!o.children.empty())
                std::snprintf(detail, sizeof(detail), "%zu", o.children.size());
            to.detail = detail;
            const ui::TreeNodeResult r = ui::TreeNode(o.name + "##" + std::to_string(i), to);
            if (r.clicked)
                selected = i;
            if (r.open)
            {
                for (const int c : objects[(std::size_t)i].children)
                    Outline(c);
                ui::TreePop();
            }
        }

        float AvailableHeight() const { return ctx->ContentRegionAvail().y / ui::S(1.0f); }

        void Frame()
        {
            ++frame;
            ui->NewFrame();
            Background();
            const Vec2 d = ctx->DisplaySize();
            const float m = ui::S(12);
            ui::DockSpace("workbench", Rect(m, m, d.x - m, d.y - m));
            if (!laidOut)
            {
                ui::DockWindow("Scene", "workbench");
                ui::DockWindow("Outline", "workbench", ui::DockSide::Left, 0.21f, "Scene");
                ui::DockWindow("Inspector", "workbench", ui::DockSide::Right, 0.27f, "Scene");
                ui::DockWindow("Assets", "workbench", ui::DockSide::Bottom, 0.36f, "Scene");
                ui::DockWindow("Script", "workbench", ui::DockSide::Center, 0.5f, "Assets");
                ui::DockWindow("Notes", "workbench", ui::DockSide::Center, 0.5f, "Assets");
                ui::FocusDockedWindow(showPanel.empty() ? "Assets" : showPanel);
                if (!floatPanel.empty())
                    ui::UndockWindow(floatPanel);
                laidOut = true;
            }

            ScenePanel();
            OutlinePanel();
            InspectorPanel();
            AssetsPanel();
            ScriptPanel();
            NotesPanel();
            ui->EndFrame();
        }

        void ScenePanel()
        {
            ui::WindowOptions wo;
            wo.icon = icon::Game;
            wo.size = Vec2(640, 420);
            wo.flags = ui::WindowFlags_NoScroll;
            if (!ui::BeginWindow("Scene", &openScene, wo))
                return;
            const Rect area(ctx->CursorPos(), ctx->CursorPos() + ctx->ContentRegionAvail() - Vec2(0, ui::S(8)));
            ctx->ItemSize(area.Size());
            Painter p = ui::GetPainter();
            p.Rect(area, Style().Radius(ui::S(14)).Fill(Paint::Linear(Color::Hex(0x0A84FF, 0.10f), Color::Hex(0xBF5AF2, 0.12f), 60)).Stroke(1.0f, Color::White(0.12f)));
            // a grid on the floor
            for (int i = 1; i < 12; ++i)
            {
                const float x = area.min.x + area.Width() * (float)i / 12.0f;
                p.FillRect(Rect(x - 0.5f, area.min.y + ui::S(6), x + 0.5f, area.max.y - ui::S(6)), Color::White(0.05f));
            }
            for (int i = 1; i < 8; ++i)
            {
                const float y = area.min.y + area.Height() * (float)i / 8.0f;
                p.FillRect(Rect(area.min.x + ui::S(6), y - 0.5f, area.max.x - ui::S(6), y + 0.5f), Color::White(0.05f));
            }
            const float unit = std::min(area.Width(), area.Height()) * 0.12f;
            for (int i = 0; i < (int)objects.size(); ++i)
            {
                Object& o = objects[(std::size_t)i];
                if (!o.shape || !o.visible)
                    continue;
                const Vec2 c(area.Center().x + o.pos[0] * area.Width() * 0.45f, area.Center().y + o.pos[1] * area.Height() * 0.45f);
                const Vec2 sz(unit * o.scale[0], unit * o.scale[1]);
                const Rect r = Rect::FromCenter(c, sz);
                const ui::Interaction it = ui::InteractRect(ctx->GetId(o.name + "##shape" + std::to_string(i)), r);
                if (it.pressed)
                    selected = i;
                Style s = Style().Radius(std::min(sz.x, sz.y) * 0.3f).Fill(o.color.Fade(0.55f * o.opacity)).Shadow(o.color.Fade(0.45f), ui::S(18), Vec2(0, ui::S(6)));
                s.Glass(ui::LookMaterial(ui::Current()->GetTheme().materials.control));
                p.PushScale(c, 1.0f + 0.05f * it.hover - 0.04f * it.press);
                p.Rect(r, s);
                if (i == selected)
                    p.Rect(r.Expanded(ui::S(4)), Style().Radius(std::min(sz.x, sz.y) * 0.3f + ui::S(4)).Stroke(ui::S(2), ui::AccentColor()));
                p.PopScale();
                const text::FontRef f = ui::Current()->Font(ui::TextStyle::Caption1);
                p.TextBox(Rect(r.min.x - ui::S(30), r.max.y + ui::S(4), r.max.x + ui::S(30), r.max.y + ui::S(20)), Vec2(0.5f, 0), f,
                          Color::White(0.85f), o.name, text::TextFlags_Ellipsis);
            }
            ui::EndWindow();
        }

        void OutlinePanel()
        {
            ui::WindowOptions wo;
            wo.icon = icon::Menu;
            wo.size = Vec2(280, 480);
            if (!ui::BeginWindow("Outline", &openOutline, wo))
                return;
            Outline(0);
            ui::EndWindow();
        }

        void InspectorPanel()
        {
            ui::WindowOptions wo;
            wo.icon = icon::Equalizer;
            wo.size = Vec2(320, 560);
            if (!ui::BeginWindow("Inspector", &openInspector, wo))
                return;
            Object& o = objects[(std::size_t)selected];
            ui::TextField("name", &o.name, "Name");
            ui::Spacer(6);
            ui::TextSecondary("Transform");
            ui::VectorField("position", o.pos, 3, {.step = 0.01, .speed = 0.004});
            ui::VectorField("rotation", o.rotation, 3, {.step = 1.0, .speed = 0.5, .format = "%.0f\xC2\xB0"});
            ui::VectorField("scale", o.scale, 3, {.min = 0.05, .max = 4.0, .step = 0.05, .speed = 0.01});
            ui::Spacer(6);
            ui::TextSecondary("Appearance");
            ui::NumberField("opacity", &o.opacity, {.min = 0.0, .max = 1.0, .step = 0.05, .format = "%.2f", .label = "Opacity"});
            ui::NumberField("layer", &o.layer, {.min = 0, .max = 31, .label = "Layer", .buttons = true});
            ui::BeginHStack("visible-row");
            ui::Text(ui::TextStyle::Body, "Visible");
            ui::FlexSpacer();
            ui::Toggle("visible", &o.visible);
            ui::EndStack();
            ui::Spacer(4);
            static const Color swatches[] = {Color::Hex(0x0A84FF), Color::Hex(0x30D158), Color::Hex(0xFF9F0A), Color::Hex(0xFF375F), Color::Hex(0xBF5AF2),
                                             Color::Hex(0x64D2FF), Color::Hex(0xFFD60A), Color::Hex(0x8E8E93)};
            ui::ColorPickerOptions co;
            co.swatches = swatches;
            ui::ColorPicker("color", &o.color, co);
            ui::EndWindow();
        }

        void AssetsPanel()
        {
            ui::WindowOptions wo;
            wo.icon = icon::Folder;
            wo.size = Vec2(720, 360);
            wo.flags = ui::WindowFlags_NoScroll;
            if (!ui::BeginWindow("Assets", &openAssets, wo))
                return;
            ui::BeginHStack("bar");
            ui::TextField("filter", &filter, "Filter assets", {.width = 320, .icon = icon::Search});
            ui::FlexSpacer();
            char count[48];
            std::snprintf(count, sizeof(count), "%zu of %zu", view.size(), assets.size());
            ui::TextSecondary("%s", count);
            ui::EndStack();
            ui::Spacer(4);
            Refilter();
            ui::TableOptions to;
            to.flags = ui::TableFlags_Sortable | ui::TableFlags_Resizable | ui::TableFlags_Selectable | ui::TableFlags_Striped;
            to.height = std::max(80.0f, AvailableHeight() - 8.0f);
            to.selection = &selection;
            if (ui::BeginTable("assets", {{"Name"}, {"Type", 120}, {"Size", 100, 0, ui::Align::End}, {"Modified", 120, 0, ui::Align::End}}, to))
            {
                sort = ui::TableSortSpec();
                if (sort.changed)
                    Refilter();
                const ui::TableRange rows = ui::TableVisible((int)view.size());
                for (int r = rows.first; r < rows.last; ++r)
                {
                    const Asset& a = assets[(std::size_t)view[(std::size_t)r]];
                    ui::TableRow(r);
                    ui::TableCellIcon(kTypeIcons[a.type], a.name);
                    ui::TableCell(kTypes[a.type], ui::Current()->GetTheme().colors.secondaryLabel);
                    char buf[32];
                    if (a.size >= 1024)
                        std::snprintf(buf, sizeof(buf), "%.1f MB", a.size / 1024.0);
                    else
                        std::snprintf(buf, sizeof(buf), "%d KB", a.size);
                    ui::TableCell(buf);
                    std::snprintf(buf, sizeof(buf), a.modified == 0 ? "today" : "%d days ago", a.modified);
                    ui::TableCell(buf, ui::Current()->GetTheme().colors.secondaryLabel);
                }
                ui::EndTable();
            }
            ui::EndWindow();
        }

        void ScriptPanel()
        {
            ui::WindowOptions wo;
            wo.icon = icon::Code;
            wo.size = Vec2(640, 360);
            wo.flags = ui::WindowFlags_NoScroll;
            if (!ui::BeginWindow("Script", &openScript, wo))
                return;
            ui::TextEditor("script", &script, {.size = Vec2(0, std::max(80.0f, AvailableHeight() - 8.0f)), .wrap = false, .lineNumbers = true, .monospace = true, .tabInput = true});
            ui::EndWindow();
        }

        void NotesPanel()
        {
            ui::WindowOptions wo;
            wo.icon = icon::Edit;
            wo.size = Vec2(420, 300);
            wo.flags = ui::WindowFlags_NoScroll;
            if (!ui::BeginWindow("Notes", &openNotes, wo))
                return;
            ui::TextEditor("notes", &notes, {.size = Vec2(0, std::max(80.0f, AvailableHeight() - 8.0f))});
            ui::EndWindow();
        }
    };
}

int main(int argc, char** argv)
{
    Demo d;
    int rows = 100000;
    glass::App app;
    app.name = "workbench";
    app.loadFonts = false;   // the Ui loads the platform's fonts
    app.option = [&](const std::string& o, const char* value, bool& usedValue) {
        if (o == "--light")
            return !(d.dark = false);
        if (!value)
            return false;
        usedValue = true;
        if (o == "--float")
        {
            d.floatPanel = value;
            return true;
        }
        if (o == "--show")
        {
            d.showPanel = value;
            return true;
        }
        if (o == "--rows")
        {
            rows = std::max(0, std::atoi(value));
            return true;
        }
        usedValue = false;
        return false;
    };
    app.init = [&](Context& ctx, text::TextSystem* text, text::FontId, const std::vector<std::string>& fonts) {
        d.ctx = &ctx;
        ui::UiDesc desc;
        desc.text = text;
        if (!fonts.empty())
            desc.fontFiles[0] = fonts[0];
        desc.theme = d.dark ? ui::ThemeDark() : ui::ThemeLight();
        desc.island = false;
#if !defined(_WIN32)
        desc.iconFontFile = glass::kSystemSymbolsFont;
#endif
        d.ui = std::make_unique<ui::Ui>(ctx, desc);
        d.Build(rows);
    };
    app.frame = [&](Context&, const glass::SceneInfo&) { d.Frame(); };
    app.shutdown = [&] { d.ui.reset(); };
    return glass::RunApp(argc, argv, app);
}
