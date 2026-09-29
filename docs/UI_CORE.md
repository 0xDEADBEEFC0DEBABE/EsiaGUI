# Esia UI core, second version

This is the design of `esia_core`'s UI machinery (`include/esia/core`, `src/esia/core`) as reworked before the WGT
widgets are ported onto it (phase 3). It covers the API, how a WGT widget maps onto it and what changes for widget
code. The architecture around the core (renderer, RHI, text) is in [REWRITE.md](REWRITE.md); what was verified is in
[REWRITE_STATUS.md](REWRITE_STATUS.md), section "UI core v2".

* [1. Goals](#1-goals)
* [2. The frame](#2-the-frame)
* [3. Input](#3-input)
* [4. Items: registration, hit testing, behavior](#4-items-registration-hit-testing-behavior)
* [5. Input routing: press ownership, keys, Tab, capture](#5-input-routing-press-ownership-keys-tab-capture)
* [6. Layout](#6-layout)
* [7. Windows](#7-windows)
* [8. Child regions and scrolling](#8-child-regions-and-scrolling)
* [9. Popups and tooltips](#9-popups-and-tooltips)
* [10. State, scopes and styling hooks](#10-state-scopes-and-styling-hooks)
* [11. Draw lists: moving commands](#11-draw-lists-moving-commands)
* [12. WGT widgets on the core](#12-wgt-widgets-on-the-core)
* [13. What changes for widget code](#13-what-changes-for-widget-code)
* [14. The review's bugs](#14-the-reviews-bugs)
* [15. Public API changes](#15-public-api-changes)

## 1. Goals

* **Immediate mode stays.** A widget is a function call per frame, as in WGT: `if (ui::Button("OK")) ...`. The
  core keeps no retained widget tree; what it remembers across frames is keyed by `Id` (hit rects, scroll offsets,
  container measurements, widget state) and dropped when unused.
* **No push / pop boilerplate in widget code.** ImGui configures widgets through global stacks (`PushStyleVar`,
  `PushItemWidth`, `PushTextWrapPos`, `PushItemFlag`, `PushID` for every row) and lets widgets poke at
  `window->DC` / `WorkRect` / `ContentRegionRect` to steer layout. Esia's replacements are scoped to what they
  describe: a container carries its layout, work rect, padding and style data from `Begin*` to `End*` and restores
  everything itself; per-item options travel with the call (`ItemFlags`, `ButtonFlags`, option structs).
* **Correct interaction by construction.** Hover is decided by hit testing last frame's item rects front to back,
  not by the order widgets happen to call `ItemHoverable`; a mouse press belongs to whoever it started over; keys
  can be owned.
* **Style-agnostic core.** The core never draws and knows no theme. It gives the widget layer what styling needs:
  an item's state (`ItemStatusOf`), per-id storage (`State<T>`), and data scoped to containers (`SetScopeData`).

## 2. The frame

`Context::NewFrame(FrameParams)`:

1. the queued input is applied to `InputState` (a second change of the same button or key waits for the next
   frame, `InputPending()` tells the host);
2. windows that were not submitted last frame lose focus, hover and drag; windows unused for
   `ContextDesc::retainFrames` frames are freed (with their child and item records), and so is `State<T>` storage;
3. per-button **press ownership** is recorded for the buttons pressed this frame (section 5);
4. window move / resize are updated, the **hit test** runs (section 4): the hovered window and the hovered item from
   last frame's rects, front to back; popups are dismissed by a click away;
5. the wheel goes to the innermost scroll area under the mouse that can scroll that way (section 8);
6. the implicit root window begins (items submitted outside any `Begin` land on it).

Widgets then submit windows and items. `Context::EndFrame()`:

1. keys nobody claimed act: Tab / Shift+Tab move keyboard focus over this frame's focusable items, Escape closes the
   top popup (section 5);
2. a click that no item took focuses the window under the mouse and starts moving it by its empty area;
3. `DrawData` (background list, windows by layer and z-order, foreground list) and `PlatformRequests` (cursor,
   capture flags, text input, IME rect, `inputPending`, `animating`).

## 3. Input

`InputState` (`include/esia/core/input.hpp`) is still fed by `InputEvent`s through `Context::QueueInput` (any
thread). Changes:

| What | API |
| --- | --- |
| No mouse | `kNoMousePos` (`-FLT_MAX`) instead of "negative". `InputEvent::MouseLeave()` queues it; `MouseValid()` tests for it. Negative coordinates are ordinary positions (drags past the left / top edge, monitors left of the primary one) |
| Focus loss | releases every button and key **and invalidates the mouse position**; the releases are marked canceled (`MouseCanceled(b)`), so nothing is clicked by a window switch |
| Click counts | `MouseClickCount(b)`: 1, 2, 3 ... for consecutive clicks within `doubleClickTime` and `doubleClickDistance` (a text editor selects words on 2, lines on 3). `MouseDoubleClicked(b)` is "clicked with a count of 2" |
| Bounds | a `MouseButton` / `Key` outside the enum is ignored by `Apply` and reads as up / zero |
| UTF-8 | `Text` events decode with `base/utf8.hpp` (`DecodeUtf8`): malformed input becomes U+FFFD. `AppendUtf8` / `AppendUtf32` are gone; `EncodeUtf8(std::string&, char32_t)` lives in `base/utf8.hpp` |

## 4. Items: registration, hit testing, behavior

```cpp
bool ItemAdd(Id id, const Rect& bb, std::uint32_t itemFlags = ItemFlags_None);   // visible?
void ItemSize(Vec2 size, float baseline = -1.0f);                                // advances the layout
bool ItemHoverable(Id id, const Rect& bb, std::uint32_t itemFlags = ItemFlags_None);
ButtonResult ButtonBehavior(Id id, const Rect& bb, std::uint32_t buttonFlags = 0, std::uint32_t itemFlags = 0);
ItemStatus ItemStatusOf(Id id) const;  ItemStatus LastItemStatus() const;
```

**Visibility and clipping.** `Begin` resets the window's draw list with the window rect as its outermost clip and
pushes the **content clip** (the content rect, i.e. the window rect minus padding, inside the window rect) on top of
it. `ItemAdd` returns whether the item overlaps the current clip, so an item scrolled into the padding (under a
title bar) is neither visible nor hoverable. Window decorations (surface, shadow, header) are drawn with an explicit
wider clip: `PushClipRect(window->GetRect(), false)`, or the shadow's extent.

**Hit testing, front to back.** Every item that `ItemAdd`s with an id, or asks `ItemHoverable` / `ButtonBehavior`,
is recorded in its window with its rect clipped to the current clip, its flags and its hit layer. At `NewFrame` the
core walks last frame's records front to back - windows by layer and z-order, inside a window floating children
above content, `ItemFlags_Background` items below the others, later-submitted above earlier-submitted (what is drawn
on top is hit on top) - and the first record under the mouse is **the hit item**. `ItemHoverable(id)` is true when
`id` is the hit item and the mouse is inside its rect *now* (an item that moved away this frame loses the hover at
once). An item that did not exist last frame may take the hover when no recorded item is under the mouse, so a new
item under a still mouse is not a frame late. This replaces the one-frame overlap trick (`AllowOverlap` yielding to
last frame's hovered id): overlapping items, rows with controls on them, floating bars and popups get the right
hover in every frame, including the frame a touch moves and presses at once. `ItemFlags_AllowOverlap` and
`ButtonFlags_AllowOverlap` are removed.

**Disabled items** (`ItemFlags_Disabled`) are hit like any item: they claim the hover (the item below does not get
it, a drag on them does not move the window), but `ItemHoverable` returns false and `ButtonBehavior` neither hovers
nor activates. An item that becomes disabled while active is deactivated by its `ItemAdd`. `ButtonBehavior` takes
the item flags; when called right after `ItemAdd` for the same id it also uses the flags given there.

**Buttons.** `ButtonBehavior` activates on a click of an enabled mouse button over the item, holds while the button
is down (`held`), and presses on release over the item (default), on click (`ButtonFlags_PressOnClick`), on a double
click (`ButtonFlags_PressOnDoubleClick`). `ButtonFlags_Repeat` presses on the click, then repeatedly after
`InputConfig::keyRepeatDelay` at `keyRepeatRate` while held over the item, and never on the release.
`ButtonResult::clicks` is the click count of the press that activated it.

**Status.** `ItemStatusOf(id)` / `LastItemStatus()` return `{hovered, active, focused, disabled, pressed, visible,
rect}` as the core knows them this frame - what a theme needs to pick a state color without the widget passing
flags around.

## 5. Input routing: press ownership, keys, Tab, capture

**Press ownership.** When a mouse button goes down, the core records who owns that press: the UI (the mouse was over
a UI window or a hit item, or on an open popup's click-away) or the host (the game). The owner keeps the press until
the release frame is over:

* a drag that starts over the game and crosses a UI window stays the game's: no item hovers or activates, and
  `wantCaptureMouse` stays false;
* a press on the UI dragged out stays the UI's: `wantCaptureMouse` stays true until the release.

`PlatformRequests::wantCaptureMouse` = some held (or just released) button is owned by the UI, or, with no button
held, the mouse is over a UI window or item, or an item is active.

**Key ownership.**

```cpp
void ClaimKey(Key key, Id owner);           // this frame and the next; renewed by claiming every frame
void ClaimKeyboard(Id owner);               // every key (a text editor while it edits)
Id   KeyOwner(Key key) const;
bool KeyPressed(Key key, Id asker = 0, bool repeat = true) const;   // false when another id owns the key
bool KeyDown(Key key, Id asker = 0) const;
```

A claim made during frame N is seen by every query after it in frame N and by all of frame N+1, so the order in
which widgets are submitted does not matter. A multiline editor claims Tab and keeps it; a dialog claims Enter and
Escape before its buttons ask for them; a popup's Escape is only used when nobody claimed it.

**Tab navigation runs last.** Tab / Shift+Tab are handled in `EndFrame`, after every widget had its chance to claim
the key, over this frame's focusable items (`ItemFlags_Focusable`, not disabled) in submission order. It does nothing
while an item is active.

**Pending input.** `Context::InputPending()` and `PlatformRequests::inputPending` are true when events were left in
the queue for the next frame (a click and its release in one frame): an event-driven host must run another frame
without waiting for a new event. `PlatformRequests::animating` is true while the core itself moves something (a
smooth scroll); widgets that animate tell the host themselves.

## 6. Layout

The layout is a stack of **containers** per window. The window's content is the root container; groups, child
regions and widget containers (rows, stacks, grids, cards, sections) nest inside. Each container has a **work rect**
(the region its current slot offers: `WorkRect()`, `ContentRegionAvail()`), a cursor, the extent of what it holds,
its depth and its scope data. A container's `End` restores its parent: no widget mutates window state to steer
layout.

```cpp
struct ContainerOptions { LayoutProvider* layout = nullptr; Vec2 size; Vec2 padding; };
void BeginContainer(Id id, const ContainerOptions& options = {});
Rect EndContainer();                        // laid out as one item of the parent; returns its rect
void BeginGroup();  void EndGroup();        // a container with the cursor layout and no id
```

**The cursor layout** (no provider) is the default: items go below each other with `LayoutMetrics::itemSpacing`,
`SameLine`, `NewLine`, `Spacing`, `Indent` work as before, `SetCursorPos` places the next item explicitly.

**Layout providers** hand out slots instead (`include/esia/core/layout.hpp`):

```cpp
struct LayoutSlot { Vec2 pos; Rect region; };    // where the next child starts, and the room it may take
class LayoutProvider {
public:
    virtual LayoutSlot Begin(Vec2 origin, const Rect& region) = 0;   // the first child's slot
    virtual LayoutSlot Next(const LaidOutItem& child) = 0;           // a child took its slot: the next one
    virtual Vec2 End(float* baseline) = 0;                           // the container's size (and baseline)
};
```

Every item laid out at the container's own depth is one child: its `ItemSize` reports it to `Next`, which returns
where the next child goes; the cursor and the work rect move there. Nested containers are single children. The
provider object belongs to the widget layer (typically in `State<T>(id)`, so it keeps last frame's measurements, as
WGT's `LayoutCache` did). The core ships one provider, `StackLayout` (vertical or horizontal, spacing, alignment
including text baselines), used by its tests and as the reference implementation; WGT's stacks, adaptive stacks,
grids and flows become providers in `esia_ui`.

**Every laid-out item is reported.** `ItemSize` produces a `LaidOutItem {rect, baseline, depth, window, container,
floating}`; it goes to the container's provider, to the observer set with `SetItemObserver(fn)` (WGT's
`WgtOnItemLaidOut` hook) and into the window's `LaidOutItems()` list, which keeps last frame's complete list (WGT's
per-window item map for glow halos and the layout inspector reads it directly).

**Baselines.** `ItemSize(size, baseline)` carries the distance from the item's top to its text baseline. The cursor
layout keeps the line's baseline (`LineBaseline()`), `AlignToLineBaseline(baseline)` moves the cursor down so an
item's baseline lines up with the items before it on the line (a label after a taller field), and `StackLayout`
aligns horizontal children on their baselines with `Align::Baseline`.

**Pixel snapping.** Positions produced by the layout (the cursor after an item, `SameLine`, slots) are rounded to
physical pixels of the window's scale (section 7): at 1.25x a 10-unit item followed by 8 units of spacing lands on
a pixel boundary, so text and hairlines stay crisp. Integer layouts at scale 1 are unchanged.

## 7. Windows

`Begin(name, WindowOptions)` / `End()` as before, plus:

* `Cond::Appearing` for `SetNextWindowPos` / `SetNextWindowSize` (applied each time the window appears);
  `SetNextWindowPos(pos, cond, pivot)`;
* `WindowFlags_AutoSize`: the window takes the size its content measured last frame (plus padding, within min /
  max); its first frame is hidden (submitted, measured, not drawn or hit), so it never flashes at a wrong size;
* **focus lifecycle**: a window that appears again comes to the front and takes focus (unless `NoFocus` /
  `NoBringToFront`); the focused, hovered and dragged window are cleared when it is not submitted; a window unused
  for `ContextDesc::retainFrames` frames is freed (a `Window*` to it dangles after that, as in any immediate-mode
  API);
* **kept on screen**: after a drag, a display size change or a DPI change, a movable window keeps at least
  `LayoutMetrics::keepOnScreen` units (and its top edge) inside the display;
* **resize** keeps the grab offset: the first frame of a resize does not snap the edge to the pointer;
* **DPI per window**: `Context::SetMonitors(std::vector<Monitor>)` (rect in UI units, scale = physical pixels per
  UI unit). A window's `Scale()` is the scale of the monitor under its center (without monitors,
  `FrameParams::framebufferScale.x`); `ScaleChanged()` is true for the frame it changed, and the window's size is
  scaled by the ratio so its content keeps its physical size. `Context::Scale()` is the current window's scale:
  widget metrics (`Theme::metrics.scale`) and the layout's pixel snapping use it.

## 8. Child regions and scrolling

```cpp
struct ChildOptions { Vec2 size; Vec2 padding; std::uint32_t flags; float clipTop = 0, clipBottom = 0; Rect rect; };
bool BeginChild(std::string_view id, const ChildOptions& options = {});   // false: clipped away (EndChild anyway)
void EndChild();
```

A child region is a container with its own clip rect and scroll offset, drawn into its window's draw list (no
separate window, so the draw order is the submission order). `size`: > 0 fixed, 0 = what is available, < 0 = what is
available minus that. `ChildFlags_ScrollX` / `ScrollY` make it a scroll area; `ChildFlags_NoWheel` keeps the wheel
out; `ChildFlags_Floating` places it at `rect` over the content without advancing the parent's layout (WGT's tab
bar and search bar) and hit-tests it above the window's other items; `clipTop` / `clipBottom` move the clip in (a
rounded window's corners). The child's rect is reported to the parent as one item (floating: flagged, depth -1).

**Scrolling** applies to the innermost scroll area being submitted (a scroll child, else the window):

| Call | Effect |
| --- | --- |
| `Scroll()`, `ScrollMax()` | the offset and the range (measured last frame) |
| `SetScrollX(x)`, `SetScrollY(y)` | deferred: applied when the area ends, clamped to the range measured **this** frame (content that grows in the same frame can be scrolled to at once) |
| `SetScrollHereX/Y(ratio)` | the cursor's position at `ratio` of the visible content (0 top, 0.5 center, 1 bottom) |
| `ScrollToRect(rect, align)`, `ScrollToItem(align)` | the smallest scroll that shows the rect (`align` < 0), or the rect at `align` of the view |
| `SetNextScroll(Vec2)` | the next `Begin` / `BeginChild` starts at this offset (a negative component is left alone) |

The wheel goes to the innermost scroll area under the mouse (from last frame's records) that can still move in that
direction, else outward to its parents and the window; Shift+wheel scrolls horizontally. With
`ChildFlags_SmoothScroll` the offset follows its target (critically damped, `LayoutMetrics::scrollSmoothing`
seconds) and snaps to physical pixels; `PlatformRequests::animating` is set while it moves. Drag-to-scroll, rubber
banding and the scroll indicator stay in the widget layer (WGT's `ScrollAreaEnd`), on `ActiveId` and
`SetScrollY`.

## 9. Popups and tooltips

```cpp
void OpenPopup(Id id);  void ClosePopup(Id id);  bool IsPopupOpen(Id id) const;
bool BeginPopup(Id id, const PopupOptions& options = {});   // a WindowLayer::Overlay window; false = closed
void EndPopup();        void CloseCurrentPopup();
bool BeginTooltip();    void EndTooltip();                  // WindowLayer::Tooltip, no inputs, at the mouse
```

* **Stack.** Open popups form a stack. `OpenPopup` inside a popup keeps the popups up to it (a submenu replaces a
  sibling submenu); outside any popup it replaces the whole stack (one menu at a time). A popup that is not
  submitted in a frame closes.
* **Placement.** `PopupOptions::pos` and `pivot` (default: the mouse position when opened, pivot top-left);
  `WindowFlags_AutoSize` by default, `minSize`; kept entirely on screen.
* **Focus.** An opened popup comes to the front of the overlay layer and takes window focus.
* **Click-away.** A click outside the popup and its child popups closes them. With
  `PopupOptions::consumeClickAway` (default) the dismissing press belongs to the popup: the item under it does not
  react until the release (iOS behavior); otherwise the click goes through (ImGui behavior).
* **Escape** closes the top popup when no item claimed it.
* **Tooltips** follow the mouse (offset `LayoutMetrics::tooltipOffset`), stay on screen, never take the hover; every
  `BeginTooltip` of a frame adds to the same tooltip window.

## 10. State, scopes and styling hooks

* **`template <class T> T& State(Id id)`**: per-id storage (springs, editor state, scroll physics, layout caches),
  default-constructed on first use, kept while used, freed after `retainFrames` unused frames. Replaces WGT's
  `impl.state.Get<T>(id)`.
* **`SetScopeData(const void* key, const void* value)` / `FindScopeData(key)`**: data attached to the innermost
  container (window, child region, container), found by walking out to the window. It ends with its container, so
  `ui::BeginCard(..., style)` attaches the card's style and every widget inside resolves it without a
  `PushItemStyle` / `PopItemStyle` pair; nothing to unbalance.
* **`ItemStatusOf(id)` / `LastItemStatus()`** (section 4): hovered, active, focused, disabled, pressed.

The widget layer keeps `Theme`, `ItemStyle` and `ui::Next()` (one pending style for the next widget, not a stack).
Resolving a widget's look is a lookup: `ui::Next()` for this widget, `FindScopeData(kStyleKey)` for its containers,
the theme, and its `ItemStatus`.

## 11. Draw lists: moving commands

A card or section draws its background after its content (its height is known then), but the background must be
under the content. ImGui used a channel splitter; Esia moves commands:

```cpp
std::size_t Mark();                                   // a position between commands (nothing merges across it)
void MoveCommands(std::size_t from, std::size_t to);  // moves commands [from, end) to position `to`
```

```cpp
const std::size_t under = dl.Mark();    // BeginCard
... content ...
const std::size_t bg = dl.Mark();       // EndCard: the background is drawn last ...
painter.Rect(card, style);
dl.MoveCommands(bg, under);             // ... and moved under the content
```

Marks nest (a card in a card) as long as they are used last-in, first-out. Vertices, indices and FX instances stay
where they are (commands address them by range), so the renderer is unchanged.

## 12. WGT widgets on the core

What WGT's `src/ui` (tag `wgt-1.1-final`) takes from ImGui, and where it goes:

| WGT (ImGui) | Esia core |
| --- | --- |
| `ImGui::ItemSize` + `ItemAdd` + `ButtonBehavior` in `InteractImpl` | the same three; `InteractFlags_AllowOverlap` has no equivalent and none is needed (hit order) |
| `PushItemFlag(ImGuiItemFlags_ButtonRepeat)` | `ButtonFlags_Repeat` |
| `WgtOnItemLaidOut` (patched `ItemSize`, `EndGroup` at the parent's depth) | `LaidOutItem` reports: providers, `SetItemObserver`, `Window::LaidOutItems()` |
| auto layout moving `DC.CursorPos`, `WorkRect`, `ContentRegionRect`, `CursorMaxPos` | a `LayoutProvider` per container (`BeginContainer`) |
| `ItemSize(size, baseline)` for text | `ItemSize(size, baseline)`, `AlignToLineBaseline` |
| cards / rows narrowing `WorkRect.Max.x` and restoring it | `BeginContainer` with a `size` / `padding` |
| `PushTextWrapPos`, `PushItemWidth` | the container's work rect (`WorkRect().max.x` is the wrap position, `ContentRegionAvail().x` the width) |
| `ImDrawListSplitter` (cards, sections) | `DrawList::Mark` / `MoveCommands` |
| `BeginChild` scroll areas with `SetNextWindowScroll` and `ScrollTarget` absorption | `BeginChild` with `ChildFlags_ScrollY` (+ `SmoothScroll`), `SetScrollY`, `SetNextScroll`; the target is the core's |
| `BeginChild` floating bars + `floatingItem` | `ChildFlags_Floating` |
| `BeginPopupEx`, `OpenPopupEx`, `CloseCurrentPopup`, `IsWindowAppearing` | `BeginPopup`, `OpenPopup`, `CloseCurrentPopup`, `Window::Appearing()` |
| `BeginTooltip` | `BeginTooltip` |
| `SetActiveIdUsingAllKeyboardKeys` | `ClaimKeyboard(id)` |
| `IsKeyPressed(key, repeat, ownerId)` | `KeyPressed(key, id)` |
| `io.MouseClickedCount` | `Input().MouseClickCount(b)` |
| `io.InputQueueCharacters` | `Input().Text()` (UTF-32) |
| `SetFocusID`, `FocusWindow`, `ActiveId` | `SetKeyboardFocusId`, `FocusWindow`, `SetActiveId` / `ActiveId` |
| `StartMouseMovingWindow` from a header | leave the header without an item: a click on empty window area moves it (or `WindowFlags_NoMove` and the widget moves it) |
| `PushStyleVar(WindowPadding / MinSize / Rounding / BorderSize ...)` around `Begin` | `WindowOptions` fields; rounding and borders are the widget's drawing |
| `PushStyleVar(Alpha)` for fades | the widget layer's style (Painter opacity), not the core |
| `impl.state.Get<T>(id)` | `State<T>(id)` |
| `PushItemStyle` / `PopItemStyle` scopes | `SetScopeData` on the container |
| `GetMainViewport()->WorkPos/WorkSize` | `DisplaySize()`, monitors |
| `Theme::metrics.scale` (one per context) | `Context::Scale()` per window |

**A button** (WGT's `InteractImpl` and `Button`):

```cpp
Interaction InteractImpl(Id id, const Rect& r, std::uint32_t flags)
{
    Context& c = Ctx();
    Interaction it;
    it.visible = c.ItemAdd(id, r, flags & InteractFlags_Disabled ? ItemFlags_Disabled : 0);
    if (!it.visible)
        return it;
    const ButtonResult b = c.ButtonBehavior(id, r, ToButtonFlags(flags));   // item flags from ItemAdd
    it.hovered = b.hovered; it.held = b.held; it.pressed = b.pressed;
    it.hover = Anim(id, 0xA1, b.hovered ? 1.0f : 0.0f, SpringFast());
    return it;
}
```

**A text field** claims the keyboard while it edits, selects on click counts, sets its own cursor:

```cpp
if (active) c.ClaimKeyboard(fid);                       // arrows, Home / End, Tab stay in the field
const int clicks = c.Input().MouseClickCount(MouseButton::Left);   // 2 = word, 3 = line / all
if (c.KeyPressed(Key::Left, fid)) ...
if (hovered || dragging) c.SetMouseCursor(MouseCursor::TextInput);
if (focused) c.RequestTextInput(caretRect);
```

**A window with a scrolling body** (WGT's `BeginWindow` / `ScrollAreaBegin`):

```cpp
WindowOptions o; o.padding = {0, 0}; o.minSize = {S(220), S(160)};
if (!c.Begin(title, o)) { c.End(); return false; }
Window* w = c.CurrentWindow();
c.PushClipRect(w->GetRect().Expanded(shadowExtent), false);   // surface + shadow outside the content clip
painter.Rect(w->GetRect(), surfaceStyle);
c.PopClipRect();
... header items ...
ChildOptions body; body.padding = {pad, pad}; body.flags = ChildFlags_ScrollY | ChildFlags_SmoothScroll;
body.clipBottom = cornerCut;
c.BeginChild("##content", body);
... content; EndWindow: c.EndChild(); c.End();
```

**A picker** (`OpenPopupEx` / `BeginGlassPopup`):

```cpp
if (it.pressed) c.OpenPopup(popupId);
PopupOptions po; po.pos = {r.max.x, r.max.y + S(6)}; po.pivot = {1, 0}; po.minSize = {std::max(r.Width(), S(180)), 0};
if (c.BeginPopup(popupId, po)) { ... rows; on a choice: c.CloseCurrentPopup(); ... c.EndPopup(); }
```

**An auto-layout container** (WGT's `BeginLayout`):

```cpp
HStackLayout& L = c.State<HStackLayout>(id);   // the widget layer's provider, keeps last frame's children
L.Configure(options);
ContainerOptions co; co.layout = &L;
c.BeginContainer(id, co);
... children: each ItemSize is one child ...
c.EndContainer();
```

## 13. What changes for widget code

* No `window->DC`, `WorkRect`, `ContentRegionRect` or `CursorMaxPos` edits: containers (`BeginContainer`,
  `BeginChild`) own that state and restore it.
* No push / pop around `Begin`, popups or rows: options structs and container scope data.
* No `AllowOverlap` flags: submit what is on top later (or mark backgrounds `ItemFlags_Background`).
* Disabled items pass `ItemFlags_Disabled` to `ItemAdd` (and get the hover claim, no activation).
* Keys are asked for with the widget's id (`KeyPressed(key, id)`) and claimed when owned.
* Scroll requests are deferred and clamped to this frame's range; smooth scrolling is a child flag.
* The text cursor is the widget's to set when hovered; `RequestTextInput` no longer sets it.
* Window decorations draw with an explicit clip: the current clip after `Begin` is the content clip.
* Draw-order tricks use `DrawList::Mark` / `MoveCommands`.
* `Context::Scale()` (per window) replaces a context-wide scale.

## 14. The review's bugs

Each has a regression test in `tests/core` (the probe scenario).

| # | Bug | Fix | Test |
| --- | --- | --- | --- |
| 1 | focus loss while holding a button clicks it | the position is invalidated, the releases are canceled | `Input.FocusLossInvalidatesMouse`, `Context.FocusLossDoesNotClick` |
| 2 | `wantCaptureMouse` ignores where a press started | press ownership per button | `Context.PressOwnershipGameDrag`, `Context.PressOwnershipUiDragOut` |
| 3 | `ContentRegionAvail().y` grows with the scroll | the work rect moves with the scroll on both axes | `Context.ContentRegionAvailIgnoresScroll` |
| 4 | repeat buttons: no press on click, extra press on release | press on click, repeats, none on release | `Context.RepeatButton` |
| 5 | disabled items: window drag, hover, flags, deactivation | disabled items are hit, flags reach `ButtonBehavior`, `ItemAdd` deactivates | `Context.DisabledItem*` |
| 6 | touch taps hit the wrong item | front-to-back hit test with this frame's position | `Context.TouchTapHitsItemUnderIt` |
| 7 | deferred input invisible to the host | `InputPending()`, `PlatformRequests::inputPending` | `Context.InputPending` |
| 8 | scrolling: stale clamp, no targets, window-wide clip | deferred clamp, `SetScrollHere*` / `ScrollTo*` / `SetNextScroll`, content clip | `Context.SetScrollClampsToThisFrame`, `Context.ScrollTargets`, `Context.ContentClip*` |
| 9 | window focus lifecycle | reappearing windows come to the front, dangling focus / drag cleared, unused windows freed | `Context.Reopened*`, `Context.WindowGc` |
| 10 | text cursor everywhere while a field has focus | `RequestTextInput` does not set the cursor | `Context.TextCursorOnlyOverField` |
| 11 | negative position = no mouse | `kNoMousePos` sentinel | `Input.NegativePositionsAreValid`, `Context.DragPastLeftEdge` |
| 12 | button index, `AppendUtf8`, resize snap, windows off screen | bounds checks, `DecodeUtf8`, grab offset, `keepOnScreen` | `Input.ButtonIndexBounds`, `Input.TextUtf8`, `Context.ResizeKeepsGrabOffset`, `Context.KeepOnScreen*` |

## 15. Public API changes

Against `main` before this work (`81ce5dc`):

* **Removed**: `ItemFlags_AllowOverlap`, `ButtonFlags_AllowOverlap`; `AppendUtf8(std::u32string&, const
  std::string&)` and `AppendUtf32(std::string&, char32_t)` (use `DecodeUtf8` / `EncodeUtf8` from
  `base/utf8.hpp`); `InputEvent::MousePos` with a negative position as "no mouse" (use `MouseLeave()`).
* **Changed**: `ItemFlags_` values (`Disabled` 1, `Focusable` 2, `Background` 4); `ItemSize(Vec2, float baseline =
  -1)`; `ButtonBehavior(id, bb, buttonFlags, itemFlags = 0)`; `ButtonResult` gains `clicks`; `SetScrollX/Y` are
  deferred; `Begin` pushes the content clip; `RequestTextInput` leaves the cursor alone; `MouseDoubleClicked` is
  "clicked with count 2" (a third click is count 3, not a new pair); Tab navigation runs in `EndFrame`;
  `FrameParams` is unchanged (monitors are set with `SetMonitors`).
* **Added**: input (`kNoMousePos`, `InputEvent::MouseLeave`, `MouseClickCount`, `MouseCanceled`), `EncodeUtf8`;
  `Cond::Appearing`, `SetNextWindowPos` pivot, `WindowFlags_AutoSize`, `Window::Hidden`, `Scale`,
  `ScaleChanged`, `LaidOutItems`, `Context::SetMonitors`, `Scale`, `InputPending`; `PlatformRequests::inputPending`,
  `animating`; `LayoutMetrics::keepOnScreen`, `scrollSmoothing`, `tooltipOffset`; `ContextDesc::retainFrames`;
  `ItemFlags_Background`, `ItemStatus`, `ItemStatusOf`, `LastItemStatus`; `ClaimKey`, `ClaimKeyboard`,
  `KeyOwner`, `KeyPressed`, `KeyDown`; containers (`BeginContainer`, `EndContainer`, `ContainerOptions`,
  `WorkRect`, `CurrentDepth`, `LineBaseline`, `AlignToLineBaseline`, `SetItemObserver`) and `layout.hpp`
  (`LayoutProvider`, `LayoutSlot`, `LaidOutItem`, `StackLayout`); child regions (`BeginChild`, `EndChild`,
  `ChildOptions`, `ChildFlags_`); scrolling (`Scroll`, `ScrollMax`, `SetScrollHereX/Y`, `ScrollToRect`,
  `ScrollToItem`, `SetNextScroll`); popups (`OpenPopup`, `ClosePopup`, `IsPopupOpen`, `BeginPopup`, `EndPopup`,
  `CloseCurrentPopup`, `PopupOptions`, `BeginTooltip`, `EndTooltip`); `State<T>`, `SetScopeData`,
  `FindScopeData`; `DrawList::Mark`, `MoveCommands`.
