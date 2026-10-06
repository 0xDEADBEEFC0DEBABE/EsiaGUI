// The debug checks (ContextDesc::debugChecks, docs/UI_CORE.md section 16): two items with one id, two child regions
// with one id, an item laid out where it cannot be seen - each reported once, outlined while it lasts.
#include "core_harness.hpp"
#include "esia_test.hpp"
#include <string>
#include <vector>

using namespace esia;
using esia::test::Harness;

namespace
{
    ContextDesc Checked(std::vector<Diagnostic>& got)
    {
        ContextDesc d;
        d.debugChecks = DebugChecks::On;
        d.diagnostics = [&got](const Diagnostic& x) { got.push_back(x); };
        return d;
    }

    bool Has(const std::string& s, const char* part) { return s.find(part) != std::string::npos; }
}

// Two items of a window with one id: reported once (with the window, the id and its label), both outlined while it
// lasts. An item's own records (ItemHoverable, then ButtonBehavior) are one item.
ESIA_TEST(DebugChecks, TwoItemsWithOneIdAreReportedOnce)
{
    std::vector<Diagnostic> got;
    Harness h(Checked(got));
    bool same = true;
    auto frame = [&] {
        h.Frame();
        h.Win("Panel", {0, 0}, {300, 200});
        h.ctx.ItemHoverable(h.ctx.GetId("Set value"), Rect(20, 20, 120, 50));
        h.ctx.ItemHoverable(h.ctx.GetId("Other"), Rect(20, 60, 120, 90));
        h.ctx.ButtonBehavior(h.ctx.GetId("Other"), Rect(20, 60, 120, 90));
        h.ctx.ItemHoverable(h.ctx.GetId(same ? "Set value" : "Set value##2"), Rect(20, 100, 120, 130));
        h.ctx.End();
        h.End();
    };
    frame();
    frame();
    frame();
    ESIA_CHECK(got.size() == 1);
    if (!got.empty())
    {
        const Diagnostic& d = got[0];
        ESIA_CHECK(d.kind == Diagnostic::Kind::IdConflict && d.window == HashLabel("Panel", 0) && d.id == HashLabel("Set value", d.window));
        ESIA_CHECK(Has(d.message, "\"Panel\"") && Has(d.message, "(\"Set value\")") && Has(d.message, "IdScope"));
        ESIA_CHECK((d.other == Rect(20, 20, 120, 50) && d.rect == Rect(20, 100, 120, 130)) ||
                   (d.rect == Rect(20, 20, 120, 50) && d.other == Rect(20, 100, 120, 130)));
    }
    ESIA_CHECK(!h.ctx.ForegroundDrawList().Empty());   // outlined
    same = false;
    frame();
    ESIA_CHECK(got.size() == 1 && h.ctx.ForegroundDrawList().Empty());
}

// Two child regions of a window begun with one id in a frame share one scroll offset: reported; each in its own
// id scope, they are two.
ESIA_TEST(DebugChecks, TwoChildRegionsWithOneIdAreReported)
{
    std::vector<Diagnostic> got;
    Harness h(Checked(got));
    bool scoped = false;
    auto frame = [&] {
        h.Frame();
        h.Win("Lists", {0, 0}, {400, 300});
        for (int i = 0; i < 2; ++i)
        {
            if (scoped)
                h.ctx.PushId(i);
            h.ctx.BeginChild("rows", {.size = {300, 100}, .flags = ChildFlags_ScrollY});
            h.ctx.ItemSize({10, 400});
            h.ctx.EndChild();
            if (scoped)
                h.ctx.PopId();
        }
        h.ctx.End();
        h.End();
    };
    scoped = true;
    frame();
    frame();
    ESIA_CHECK(got.empty());
    scoped = false;
    frame();
    frame();
    ESIA_CHECK(got.size() == 1 && got[0].kind == Diagnostic::Kind::ChildIdConflict && Has(got[0].message, "(\"rows\")"));
}

// A row wider than its window: the item past the edge, which nothing scrolls sideways, is reported once it has
// stayed there a second (by its id and label); items scrolled out of a window's view are not, nor is an item that
// passes out of view and back. In a window that does not scroll, an item below its bottom is.
ESIA_TEST(DebugChecks, AnItemLaidOutOutOfViewIsReported)
{
    std::vector<Diagnostic> got;
    Harness h(Checked(got));
    float field = 200.0f;   // the content is 12 .. 288 across (padding 12)
    auto frame = [&] {
        h.Frame(0.1);
        h.Win("Row", {0, 0}, {300, 200});
        h.ctx.ItemSize({field, 30});
        h.ctx.SameLine();
        const Vec2 at = h.ctx.CursorPos();
        h.ctx.ItemSize({60, 30});
        h.ctx.ItemAdd(h.ctx.GetId("Set value"), Rect::FromSize(at, {60, 30}));
        for (int i = 0; i < 20; ++i)
            h.ctx.ItemSize({100, 30});   // below the bottom: scrolled to
        h.ctx.End();
        h.Win("Fixed", {320, 0}, {200, 100}, {.flags = WindowFlags_NoScroll});   // content 12 .. 88 down
        h.ctx.ItemSize({100, 60});
        h.ctx.ItemSize({100, 60});   // half in view
        h.ctx.ItemSize({100, 60});   // below a bottom nothing scrolls to
        h.ctx.End();
        h.End();
    };
    for (int i = 0; i < 9; ++i)
        frame();   // out of view for 0.8 s
    ESIA_CHECK(got.empty());
    for (int i = 0; i < 3; ++i)
        frame();
    ESIA_CHECK(got.size() == 1);   // the item below "Fixed"'s bottom
    if (got.size() == 1)
        ESIA_CHECK(got[0].kind == Diagnostic::Kind::OutOfView && got[0].window == HashLabel("Fixed", 0) && got[0].id == 0 &&
                   Has(got[0].message, "bottom edge"));
    field = 280.0f;   // "Set value" starts at 300: past the right edge
    for (int i = 0; i < 5; ++i)
        frame();
    field = 200.0f;   // back in view: it passed through
    frame();
    field = 280.0f;
    for (int i = 0; i < 9; ++i)
        frame();
    ESIA_CHECK(got.size() == 1);
    for (int i = 0; i < 3; ++i)
        frame();
    ESIA_CHECK(got.size() == 2);
    if (got.size() == 2)
    {
        const Diagnostic& d = got[1];
        ESIA_CHECK(d.kind == Diagnostic::Kind::OutOfView && d.id == HashLabel("Set value", HashLabel("Row", 0)) && d.rect.min.x == 300.0f);
        ESIA_CHECK(Has(d.message, "\"Row\"") && Has(d.message, "(\"Set value\")") && Has(d.message, "right edge"));
    }
    ESIA_CHECK(!h.ctx.ForegroundDrawList().Empty());
    for (int i = 0; i < 20; ++i)
        frame();
    ESIA_CHECK(got.size() == 2);   // once
}

// Off, nothing is checked; SetDebugChecks switches them at run time
ESIA_TEST(DebugChecks, OffChecksNothing)
{
    std::vector<Diagnostic> got;
    ContextDesc desc = Checked(got);
    desc.debugChecks = DebugChecks::Off;
    Harness h(desc);
    auto frame = [&] {
        h.Frame();
        h.Win("Panel", {0, 0}, {300, 200});
        for (const Rect& r : {Rect(20, 20, 120, 50), Rect(20, 100, 120, 130)})   // two items, one after the other
        {
            h.ctx.ItemAdd(h.ctx.GetId("a"), r);
            h.ctx.ButtonBehavior(h.ctx.GetId("a"), r);
        }
        h.ctx.End();
        h.End();
    };
    frame();
    ESIA_CHECK(!h.ctx.DebugChecksOn() && got.empty() && h.ctx.ForegroundDrawList().Empty());
    h.ctx.SetDebugChecks(true);
    frame();
    ESIA_CHECK(h.ctx.DebugChecksOn() && got.size() == 1);
}

// A string literal is a label (hashed, as GetId's), not a pointer; any integer type is a value
ESIA_TEST(Context, PushIdTakesLiteralsAsLabels)
{
    Harness h;
    h.Frame();
    h.Win("Ids", {0, 0}, {300, 200});
    const Id seed = h.ctx.IdSeed();
    h.ctx.PushId("row");
    ESIA_CHECK(h.ctx.IdSeed() == HashLabel("row", seed));
    h.ctx.PopId();
    const std::size_t index = 3;
    h.ctx.PushId(index);
    const Id a = h.ctx.IdSeed();
    h.ctx.PopId();
    h.ctx.PushId(3);
    ESIA_CHECK(h.ctx.IdSeed() == a && a == HashInt(3, seed));
    h.ctx.PopId();
    ESIA_CHECK(h.ctx.GetId(0) == HashInt(0, seed) && h.ctx.GetId("x") == HashLabel("x", seed));
    h.ctx.End();
    h.End();
}
