#include "esia/base/utf8.hpp"
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
        EncodeUtf8(back, c);
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

// review bug 1: a window switch mid-press must not look like "released over the item"
ESIA_TEST(Input, FocusLossInvalidatesMouse)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::MouseMove({10, 10}), InputEvent::Button(MouseButton::Left, true)});
    ESIA_CHECK(in.MouseValid() && in.MouseDown(MouseButton::Left));
    Frame(in, 1.1, {InputEvent::FocusEvent(false)});
    ESIA_CHECK(!in.MouseValid());
    ESIA_CHECK(in.MouseReleased(MouseButton::Left) && in.MouseCanceled(MouseButton::Left));
    ESIA_CHECK(in.MouseClickCount(MouseButton::Left) == 0);
    // back: the next move makes the position valid again, and nothing is canceled any more
    Frame(in, 1.2, {InputEvent::FocusEvent(true), InputEvent::MouseMove({12, 12})});
    ESIA_CHECK(in.MouseValid() && in.MousePos() == Vec2(12, 12));
    ESIA_CHECK(!in.MouseCanceled(MouseButton::Left) && !in.MouseReleased(MouseButton::Left));
}

// review bug 11: negative coordinates are positions (drags past the left / top edge, monitors to the left)
ESIA_TEST(Input, NegativePositionsAreValid)
{
    InputState in;
    ESIA_CHECK(!in.MouseValid());   // no mouse before the first move
    Frame(in, 1.0, {InputEvent::MouseMove({-50, -20})});
    ESIA_CHECK(in.MouseValid() && in.MousePos() == Vec2(-50, -20));
    Frame(in, 1.1, {InputEvent::MouseMove({-60, -20})});
    ESIA_CHECK(in.MouseDelta() == Vec2(-10, 0));
    Frame(in, 1.2, {InputEvent::MouseLeave()});
    ESIA_CHECK(!in.MouseValid() && in.MouseDelta() == Vec2(0, 0));
    Frame(in, 1.3, {InputEvent::MouseMove({5, 5})});
    ESIA_CHECK(in.MouseDelta() == Vec2(0, 0));   // no delta across a leave
}

// review bug 12: out-of-range buttons and keys (a cast from a platform code) are ignored, not out of bounds
ESIA_TEST(Input, ButtonIndexBounds)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::Button((MouseButton)9, true), InputEvent::KeyEvent((Key)999, true)});
    ESIA_CHECK(!in.MouseDown((MouseButton)9) && !in.MouseClicked((MouseButton)200));
    ESIA_CHECK(in.MouseDownDuration((MouseButton)9) < 0.0f && in.MouseClickCount((MouseButton)9) == 0);
    ESIA_CHECK(!in.KeyDown((Key)999) && !in.KeyPressed((Key)999) && !in.KeyReleased((Key)999));
    for (int b = 0; b < (int)MouseButton::Count; ++b)
        ESIA_CHECK(!in.MouseDown((MouseButton)b));
}

// review bug 12: text decodes with base/utf8.hpp: malformed and truncated sequences become U+FFFD
ESIA_TEST(Input, TextUtf8)
{
    InputState in;
    Frame(in, 1.0, {InputEvent::TextEvent("a\xFF"), InputEvent::TextEvent("\xE2\x82"), InputEvent::TextEvent("\xF0\x9F\x98\x80")});
    ESIA_CHECK(in.Text() == U"a���\U0001F600");
    std::string s;
    EncodeUtf8(s, 0xD800);   // a surrogate cannot be encoded
    EncodeUtf8(s, 0x110000);
    ESIA_CHECK(s == "\xEF\xBF\xBD\xEF\xBF\xBD");
}

ESIA_TEST(Input, ClickCounts)
{
    InputState in;
    double t = 1.0;
    auto click = [&](Vec2 p) {
        Frame(in, t, {InputEvent::MouseMove(p), InputEvent::Button(MouseButton::Left, true)});
        const int n = in.MouseClickCount(MouseButton::Left);
        t += 0.05;
        Frame(in, t, {InputEvent::Button(MouseButton::Left, false)});
        t += 0.05;
        return n;
    };
    ESIA_CHECK(click({5, 5}) == 1);
    ESIA_CHECK(click({6, 5}) == 2);
    ESIA_CHECK(click({6, 6}) == 3);   // a triple click (a text editor selects the line)
    ESIA_CHECK(click({6, 6}) == 4);
    ESIA_CHECK(click({60, 6}) == 1);  // too far: a new sequence
    t += 1.0;
    ESIA_CHECK(click({60, 6}) == 1);  // too late
    Frame(in, t, {InputEvent::Button(MouseButton::Left, true)});
    t += 0.05;
    ESIA_CHECK(in.MouseDoubleClicked(MouseButton::Left) && in.MouseClickCount(MouseButton::Left) == 2);
    Frame(in, t, {});
    ESIA_CHECK(!in.MouseDoubleClicked(MouseButton::Left) && in.MouseClickCount(MouseButton::Left) == 2);   // kept while held
}
