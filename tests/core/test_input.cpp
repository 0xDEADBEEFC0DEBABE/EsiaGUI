#include "esia/core/input.hpp"
#include "esia_test.hpp"

using namespace esia;

namespace
{
    void Frame(InputState& in, double t, std::vector<InputEvent> ev)
    {
        in.NewFrame(t, ev);
    }
}

ESIA_TEST(Input, ClickAndRelease)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::MouseMove({10, 10}), InputEvent::Button(MouseButton::Left, true)});
    ESIA_CHECK(in.MouseClicked(MouseButton::Left) && in.MouseDown(MouseButton::Left));
    Frame(in, 1.016, {});
    ESIA_CHECK(!in.MouseClicked(MouseButton::Left) && in.MouseDown(MouseButton::Left));
    Frame(in, 1.032, {InputEvent::Button(MouseButton::Left, false)});
    ESIA_CHECK(in.MouseReleased(MouseButton::Left) && !in.MouseDown(MouseButton::Left));
}

ESIA_TEST(Input, FastClickTricklesToNextFrame)
{
    InputState in;
    std::vector<InputEvent> ev = {InputEvent::MouseMove({5, 5}), InputEvent::Button(MouseButton::Left, true),
                                  InputEvent::Button(MouseButton::Left, false), InputEvent::MouseMove({50, 50})};
    in.NewFrame(1.0, ev);
    ESIA_CHECK(in.MouseClicked(MouseButton::Left));
    ESIA_CHECK(ev.size() == 2);   // the release and the move wait
    ESIA_CHECK(in.MousePos() == Vec2(5, 5));
    in.NewFrame(1.016, ev);
    ESIA_CHECK(in.MouseReleased(MouseButton::Left));
    ESIA_CHECK(in.MousePos() == Vec2(5, 5));   // the release happened where the press did
    in.NewFrame(1.032, ev);
    ESIA_CHECK(ev.empty());
    ESIA_CHECK(in.MousePos() == Vec2(50, 50));
}

ESIA_TEST(Input, DoubleClick)
{
    InputState in;
    Frame(in, 1.00, {InputEvent::MouseMove({5, 5}), InputEvent::Button(MouseButton::Left, true)});
    Frame(in, 1.05, {InputEvent::Button(MouseButton::Left, false)});
    Frame(in, 1.10, {InputEvent::Button(MouseButton::Left, true)});
    ESIA_CHECK(in.MouseDoubleClicked(MouseButton::Left));
    Frame(in, 1.15, {InputEvent::Button(MouseButton::Left, false)});
    Frame(in, 1.20, {InputEvent::Button(MouseButton::Left, true)});
    ESIA_CHECK(!in.MouseDoubleClicked(MouseButton::Left));   // a third click starts over
    Frame(in, 1.25, {InputEvent::Button(MouseButton::Left, false)});
    Frame(in, 2.00, {InputEvent::Button(MouseButton::Left, true)});
    ESIA_CHECK(!in.MouseDoubleClicked(MouseButton::Left));   // too late
}

ESIA_TEST(Input, DragThreshold)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::MouseMove({0, 0}), InputEvent::Button(MouseButton::Left, true)});
    Frame(in, 1.1, {InputEvent::MouseMove({3, 0})});
    ESIA_CHECK(!in.MouseDragging(MouseButton::Left));
    ESIA_CHECK(in.MouseDragDelta(MouseButton::Left) == Vec2(0, 0));
    ESIA_CHECK(in.MouseDragDelta(MouseButton::Left, 0.0f) == Vec2(3, 0));
    Frame(in, 1.2, {InputEvent::MouseMove({10, 4})});
    ESIA_CHECK(in.MouseDragging(MouseButton::Left));
    ESIA_CHECK(in.MouseDragDelta(MouseButton::Left) == Vec2(10, 4));
    ESIA_CHECK(in.MouseDelta() == Vec2(7, 4));
}

ESIA_TEST(Input, KeyRepeat)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::KeyEvent(Key::Backspace, true)});
    ESIA_CHECK(in.KeyPressed(Key::Backspace));
    Frame(in, 1.1, {});
    ESIA_CHECK(!in.KeyPressed(Key::Backspace));
    ESIA_CHECK(!in.KeyPressed(Key::Backspace, false));
    int repeats = 0;
    for (int i = 0; i < 60; ++i)   // one second at 60 Hz past the first frame
    {
        Frame(in, 1.1 + (i + 1) / 60.0, {});
        repeats += in.KeyPressed(Key::Backspace) ? 1 : 0;
    }
    // delay 0.275 s then every 0.05 s up to t = 1.1 + 1.0 -> about 16 repeats (one per frame at most)
    ESIA_CHECK(repeats >= 14 && repeats <= 18);
    Frame(in, 3.0, {InputEvent::KeyEvent(Key::Backspace, false)});
    ESIA_CHECK(in.KeyReleased(Key::Backspace));
}

ESIA_TEST(Input, TextImeAndMods)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::TextEvent("a\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"), InputEvent::Composition("ni", 2),
                    InputEvent::KeyEvent(Key::LeftShift, true, Mod_Shift)});
    ESIA_CHECK(in.Text() == U"aé中\U0001F600");
    ESIA_CHECK(in.Composition() == "ni" && in.CompositionCursor() == 2);
    ESIA_CHECK(in.Mods() == Mod_Shift);
    Frame(in, 1.1, {InputEvent::Composition("", 0)});
    ESIA_CHECK(in.Text().empty() && in.Composition().empty());
    std::string back;
    for (char32_t c : std::u32string(U"aé中\U0001F600"))
        AppendUtf32(back, c);
    ESIA_CHECK(back == "a\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80");
}

ESIA_TEST(Input, FocusLossReleasesEverything)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::Button(MouseButton::Right, true), InputEvent::KeyEvent(Key::A, true, Mod_Ctrl)});
    Frame(in, 1.1, {InputEvent::FocusEvent(false)});
    ESIA_CHECK(!in.MouseDown(MouseButton::Right) && in.MouseReleased(MouseButton::Right));
    ESIA_CHECK(!in.KeyDown(Key::A) && in.Mods() == 0 && !in.Focused());
}

ESIA_TEST(Input, DeltaTimeFromAClockStartingAtZero)
{
    InputState in;
    Frame(in, 0.0, {});
    ESIA_CHECK(in.DeltaTime() == 0.0f);
    Frame(in, 0.016, {});
    ESIA_CHECK_NEAR(in.DeltaTime(), 0.016f, 1e-6f);
    Frame(in, 0.010, {});   // a clock that went backwards
    ESIA_CHECK(in.DeltaTime() == 0.0f);
}
