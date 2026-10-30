// The widgets' draw memos (UiDesc::drawMemo) against drawing every widget: the same draw data, byte for byte, frame
// after frame - at rest, through hovers, presses, clicks and drags, a style on one widget, a theme's transition,
// another scale, content that moves by fractions of a pixel (its text in motion, then settling) - with the FreeType text system and the test
// fonts; and the "verify" mode finds no drawing that differs from what would have been appended.
#include "esia/text/freetype.hpp"
#include "esia/ui/ui.hpp"
#include "esia_test.hpp"
#include "ui_internal.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

using namespace esia;

namespace
{
    const std::string kFonts = ESIA_TEST_FONT_DIR;

    // A Ui on a context and a FreeType text system of its own; memo: 0 off, 1 on, 2 verify
    struct App
    {
        Context ctx;
        std::unique_ptr<text::TextSystem> ts;
        std::unique_ptr<ui::Ui> ui;
        bool toggles[6] = {true, false, true, false, true, false};
        float values[6] = {0.10f, 0.25f, 0.40f, 0.55f, 0.70f, 0.85f};
        bool rowToggle = true;
        bool check = false;
        float rowValue = 40.0f;

        App(int memo, bool smoothMotion)
        {
            text::FreeTypeDesc fd;
            if (smoothMotion)
                fd.rendering = text::TextRendering::Light();   // moving text is drawn in motion, then settles
            ts = text::CreateFreeTypeTextSystem(ctx.Textures(), fd);
            ui::UiDesc d;
            d.text = ts.get();
            for (std::string& f : d.fontFiles)
                f = kFonts + "/DroidSans.ttf";
            d.iconFontFile = kFonts + "/Karla-Regular.ttf";   // (its letters stand for icons)
            d.fallbackChain = false;
            d.island = false;
            d.theme = ui::ThemeDark();
            ui = std::make_unique<ui::Ui>(ctx, d);
            ui->GetImpl().memoMode = memo;   // (over ESIA_UI_MEMO)
        }
    };

    // What a frame of the script does to every app alike
    struct Step
    {
        Vec2 mouse{-1e6f, -1e6f};
        int button = -1;           // 1 down, 0 up
        float shift = 0.0f;        // a spacer above the rows: everything below moves
        Vec2 scale{1, 1};
        bool flat = false;
        int styledRow = -1;        // a Next() tint on that row's button
        bool dark = true;
    };

    void Frame(App& a, const Step& s, double t)
    {
        if (s.mouse.x > -1e5f)
            a.ctx.QueueInput(InputEvent::MouseMove(s.mouse));
        if (s.button >= 0)
            a.ctx.QueueInput(InputEvent::Button(MouseButton::Left, s.button == 1));
        a.ui->SetFlat(s.flat);
        a.ctx.NewFrame({Vec2(800, 1400), s.scale, t});
        a.ui->NewFrame();
        ui::WindowOptions wo;
        wo.pos = Vec2(20, 16);
        wo.size = Vec2(380, 1300);
        if (ui::BeginWindow("Panel", nullptr, wo))
        {
            ui::Spacer(1.0f + s.shift);
            static const char* rows[6] = {"r0", "r1", "r2", "r3", "r4", "r5"};
            static const char* runs[6] = {"Run 1", "Run 2", "Run 3", "Run 4", "Run 5", "Run 6"};
            static const char* tg[6] = {"t0", "t1", "t2", "t3", "t4", "t5"};
            static const char* sl[6] = {"s0", "s1", "s2", "s3", "s4", "s5"};
            for (int r = 0; r < 6; ++r)
            {
                ui::BeginHStack(rows[r]);
                ui::Text(ui::TextStyle::Body, "Row %d", r + 1);
                if (r == s.styledRow)
                    ui::Next().Tint(Color(1.0f, 0.35f, 0.2f));
                ui::Button(runs[r]);
                ui::Toggle(tg[r], &a.toggles[r]);
                ui::Slider(sl[r], &a.values[r], 0.0f, 1.0f);
                ui::EndStack();
            }
            ui::IconButton("icon", (ui::Icon)'Q');
            ui::BeginHStack("extras");
            ui::Checkbox("Remember", &a.check);
            ui::Badge("12");
            ui::ProgressRing(a.values[0]);
            ui::EndStack();
            ui::ProgressBar(a.values[1]);
            if (ui::BeginSection("Settings", "A footer under the rows."))
            {
                ui::RowValue("Version", "1.2.3");
                if (s.styledRow == 2)
                    ui::Next().Tint(Color(0.2f, 0.6f, 1.0f));
                ui::RowToggle("Wi-Fi", &a.rowToggle, ui::RowIcon{(ui::Icon)'W', Color(0.2f, 0.5f, 1.0f)});
                ui::RowSlider("Volume", &a.rowValue, 0.0f, 100.0f, ui::RowIcon{}, "%.0f");
                ui::RowNavigation("General", "Details");
                ui::RowButton("Sign out", true);
                ui::EndSection();
            }
            static const ui::TableColumn kCols[] = {{"Name"}, {"Size", 80, 0, ui::Align::End}, {"Kind", 90}};
            static const char* names[4] = {"alpha.png", "beta.wav", "gamma.txt", "delta.bin"};
            static const char* sizes[4] = {"12 KB", "3.4 MB", "1 KB", "88 KB"};
            static const char* kinds[4] = {"Image", "Sound", "Text", "Data"};
            if (ui::BeginTable("files", std::span<const ui::TableColumn>(kCols)))
            {
                const ui::TableRange range = ui::TableVisible(4);
                for (int r = range.first; r < range.last; ++r)
                {
                    ui::TableRow(r);
                    ui::TableCellIcon((ui::Icon)'F', names[r]);
                    ui::TableCell(sizes[r]);
                    ui::TableCell(kinds[r], Color(0.6f, 0.6f, 0.65f));
                }
                ui::EndTable();
            }
            ui::TreeNodeOptions to;
            to.flags = ui::TreeFlags_DefaultOpen | (s.styledRow >= 0 ? ui::TreeFlags_Selected : 0u);
            to.detail = "2";
            if (ui::TreeNode("Scene", to).open)
            {
                ui::TreeNodeOptions leaf;
                leaf.flags = ui::TreeFlags_Leaf;
                leaf.icon = (ui::Icon)'L';
                ui::TreeNode("Light", leaf);
                ui::TreeNode("Camera", leaf);
                ui::TreePop();
            }
            ui::TextWrapped(ui::TextStyle::Body, Color(0.92f, 0.92f, 0.95f),
                            "Draw memos keep a widget's drawing from the frame before and append it again while nothing it "
                            "is drawn from has changed: the same output, byte for byte.");
            ui::EndWindow();
        }
        a.ui->EndFrame();
        a.ctx.EndFrame();
    }

    bool SameList(const DrawList& a, const DrawList& b)
    {
        const auto bytes = [](const auto& x, const auto& y) {
            return x.size() == y.size() && (x.empty() || std::memcmp(x.data(), y.data(), x.size() * sizeof(x[0])) == 0);
        };
        if (!bytes(a.Vertices(), b.Vertices()) || !bytes(a.Indices(), b.Indices()) || !bytes(a.Quads(), b.Quads()) ||
            !bytes(a.FxInstances(), b.FxInstances()) || !bytes(a.Layers(), b.Layers()) || !bytes(a.Fades(), b.Fades()) ||
            a.Commands().size() != b.Commands().size())
            return false;
        for (std::size_t i = 0; i < a.Commands().size(); ++i)
        {
            const DrawCmd &x = a.Commands()[i], &y = b.Commands()[i];
            if (x.kind != y.kind || !(x.clip == y.clip) || x.texture != y.texture || x.anyTexture != y.anyTexture || x.shapes != y.shapes ||
                x.first != y.first || x.count != y.count || x.effect != y.effect || x.payload != y.payload || x.vtxFirst != y.vtxFirst ||
                x.vtxEnd != y.vtxEnd || !(x.vtxBounds == y.vtxBounds))
                return false;
        }
        return true;
    }

    bool SameFrame(const App& a, const App& b)
    {
        const DrawData &x = a.ctx.GetDrawData(), &y = b.ctx.GetDrawData();
        if (x.lists.size() != y.lists.size())
            return false;
        for (std::size_t i = 0; i < x.lists.size(); ++i)
            if (!SameList(*x.lists[i], *y.lists[i]))
                return false;
        return true;
    }

    // The script: still, the mouse across the rows with a click and a drag, still, a style on a button, the theme's
    // transition to light and back, a second scale, the content moving and settling, the flat look.
    Step ScriptStep(int f)
    {
        Step s;
        if (f >= 30 && f < 110)
        {
            const float k = (float)(f - 30) / 80.0f;
            s.mouse = Vec2(40.0f + 300.0f * k, 40.0f + 1180.0f * k);   // down to the section's rows, the table and the tree
            if (f == 50 || f == 80)
                s.button = 1;
            if (f == 53 || f == 95)
                s.button = 0;
        }
        else if (f >= 110)
            s.mouse = Vec2(700, 1360);   // out of the window
        if (f >= 140 && f < 170)
            s.styledRow = 2;
        if (f >= 180 && f < 260)
            s.dark = f >= 220;   // light at 180, dark again at 220 (each a transition)
        if (f >= 270 && f < 320)
            s.scale = Vec2(2, 2);
        if (f >= 330 && f < 345)
            s.shift = 0.61f * (float)(f - 329);
        else if (f >= 345)
            s.shift = 0.61f * 16.0f;
        s.flat = f >= 380 && f < 420;
        return s;
    }

    void RunScript(bool smoothMotion)
    {
        App on(1, smoothMotion), off(0, smoothMotion), verify(2, smoothMotion);
        if (!on.ts || !off.ts || !verify.ts)
            return;   // no FreeType: nothing to draw text with
        int differ = 0;
        bool dark = true;
        for (int f = 0; f < 440; ++f)
        {
            const Step s = ScriptStep(f);
            if (s.dark != dark)
            {
                for (App* a : {&on, &off, &verify})
                    a->ui->SetDarkMode(s.dark, true);
                dark = s.dark;
            }
            const double t = 1.0 + f / 60.0;
            Frame(on, s, t);
            Frame(off, s, t);
            Frame(verify, s, t);
            if (!SameFrame(on, off) && ++differ <= 4)
                std::fprintf(stderr, "  frame %d: the memos' draw data differs from drawing every widget\n", f);
        }
        ESIA_CHECK(differ == 0);
        const ui::DrawMemoStats st = on.ui->MemoStats(), sv = verify.ui->MemoStats();
        std::printf("  smooth motion %d: replayed %llu, drawn %llu; verify: drawn %llu, mismatches %llu\n", smoothMotion ? 1 : 0,
                    (unsigned long long)st.replayed, (unsigned long long)st.drawn, (unsigned long long)sv.drawn, (unsigned long long)sv.mismatches);
        ESIA_CHECK(sv.mismatches == 0);
        ESIA_CHECK(st.replayed > st.drawn);   // most frames are still: most widgets appended again
        ESIA_CHECK(off.ui->MemoStats().replayed == 0 && off.ui->MemoStats().drawn == 0);
    }
}

ESIA_TEST(DrawMemo, TheSameDrawDataAsDrawingEveryWidget)
{
    RunScript(false);
}

ESIA_TEST(DrawMemo, TheSameDrawDataWithTextInMotion)
{
    RunScript(true);
}
