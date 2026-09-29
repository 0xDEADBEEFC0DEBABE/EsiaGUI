# Threading

WGT follows one rule: **exactly one thread builds and renders the UI; every other thread talks to it through
thread-safe services.** Nothing is ever "sometimes safe": each API is either UI-thread only or safe from any
thread, and the headers mark which.

## The UI thread

The thread that calls `Context::Create()` (and then `NewFrame()` / `Render()`) is the context's UI thread.
It is usually your render thread. On that thread, between `NewFrame()` and `Render()`, you may call:

* every `wgt::ui::*` widget, `wgt::Painter`, `wgt::anim::*`;
* `ImGui::*` (the ImGui context pointer is thread-local; see below);
* `CurrentTheme()`, `Context::GetTheme()`, `GetFont()`, `MeasureText()`, `RegisterFont()`;
* `Context::InvalidateDeviceObjects()` (outside the frame).

Panel callbacks (`AddPanel`), tasks (`Post`) and plugin hooks (`OnAttach`, `OnFrame`, `OnDetach`) are
always *invoked* on the UI thread, inside the frame. They are UI code even when they were registered
from another thread.

## Any thread

These `Context` methods can be called concurrently from game, worker, network or window threads. They
are queued or protected by locks and atomics, and take effect at the next `NewFrame()`:

| Area | Methods |
| --- | --- |
| input | `HandleWin32Message`, `WantsMouse`, `WantsKeyboard`, `Inject*`, `EndInputInjection` |
| visibility | `SetVisible`, `IsVisible`, `ToggleVisible` |
| work | `Post(task)`: runs `task` on the UI thread at the start of the next frame |
| look | `SetTheme`, `SetDarkMode`, `IsDarkMode`, `SetAccent`, `SetUiScale`, `SetDpiScale`, `SetRenderScale`, `GetScaleInfo` |
| overlay | `Notify`, `SetActivity`, `ClearActivity` |
| panels | `AddPanel`, `RemovePanel`, `SetPanelOpen`, `IsPanelOpen`, `SetDockVisible` |
| resources | `RegisterEffect`, `AddFontFile`, `CreateTexture`, `DestroyTexture` |
| plugins | `LoadPlugin`, `LoadPluginsFromDirectory`, `AddPlugin` (attached on the UI thread at the next frame) |
| misc | `SetLogCallback`, `GetStats`, `GetBackend` |

The log callback can be invoked from whichever thread produced the message. Invocations are serialized.

## Window thread vs render thread

Many engines pump Win32 messages on one thread and render on another. WGT supports this directly:

* `HandleWin32Message()` copies input messages into a queue and returns immediately. Its return value
  (capture) is based on the previous frame's `WantsMouse()` / `WantsKeyboard()`.
* `NewFrame()` replays the queue in order on the UI thread.
* The mouse cursor shape is published atomically and applied by `HandleWin32Message` on `WM_SETCURSOR`.
* DPI changes (`WM_DPICHANGED`) are flagged atomically. The UI thread re-reads the monitor DPI at the
  next frame.
* **Keyboard state is per thread** in Win32: `GetKeyState` answers for the calling thread's message
  queue. So `HandleWin32Message` captures the keyboard state with every key and focus message, and the
  replay feeds that snapshot to the Win32 backend. Modifiers (Ctrl, Shift, Alt) are therefore correct on
  any thread.
* **Mouse capture and leave tracking** (`SetCapture`, `TrackMouseEvent`) only work on the window's thread.
  `HandleWin32Message` does them there, so dragging a slider outside the window keeps working.
* **IME** (`ImmGetContext` and friends) is window-thread only. Composition and result strings are read in
  `HandleWin32Message`, and the candidate window is positioned there from the caret position the UI
  thread publishes.

The demo runs this way. Its main thread creates the window and blocks in `GetMessage`. A render thread
creates the device and the context and runs the frame loop, so moving or resizing the window never
stalls the UI.

## Moving data to the UI (`wgt/sync.hpp`)

Header-only, lock-free where possible, and safe across DLLs:

```cpp
// Property<T>: an observable value. Lock-free for small trivially copyable T (bool, int, float, Vec2...).
wgt::Property<float> health{1.0f};
health.Set(0.75f);                                     // game thread
health.Update([](float& v) { v -= 0.1f; });            // atomic read-modify-write
float h = health.Get();                                // UI thread
static std::uint64_t seen = 0;
if (health.Changed(seen)) { /* react once per change */ }

// Channel<T>: multi-producer / single-consumer queue (events, log lines, commands).
wgt::Channel<std::string> log;
log.Push("connected");                                 // any thread
log.Drain([](std::string& line) { lines.push_back(std::move(line)); });   // UI thread, each frame

// Latest<T>: lock-free triple buffer. The producer publishes snapshots, the UI always reads the newest.
wgt::Latest<Telemetry> telemetry;
telemetry.Publish(sample);                             // producer thread
Telemetry t;
if (telemetry.Fetch(t)) { /* new sample */ }           // consumer (UI) thread
```

For anything else, `ctx->Post([=] { ... })` runs a closure on the UI thread.

## Several contexts

Dear ImGui's current-context pointer is `thread_local` in `wgt.dll`, and each `wgt::Context` owns its own
ImGui context, theme, fonts, animation store and backend objects. Two contexts can therefore run on two
threads at the same time, e.g. an in-game HUD and a tool window. `NewFrame()` makes its context current
on the calling thread. Use `SetCurrentContext()` to switch explicitly when one thread drives several
contexts.

## GPU work

`Render()` runs on the UI thread:

* **D3D11:** it uses the immediate context given in `ContextDesc` and restores the host's pipeline state
  afterwards, unless `d3d11RestoreState = false`.
* **D3D12:** it records into the host's command list. Submission and fencing stay with the host.
  `d3d12FramesInFlight` must be at least the number of frames your renderer keeps in flight, because
  per-frame upload memory and descriptors are recycled on that cadence.

Custom effect shaders compile on a worker thread, started as soon as the effect is registered: `D3DCompile`,
D3D11 shader creation and D3D12 bytecode are free-threaded. A compile takes about a second (the effect is
spliced into the full FX shader). Until the shader is ready the shape renders with the built-in shader, so
registering an effect never causes a frame hitch, and `RegisterEffect` returns immediately on any thread.

Hosts should wait for their back buffer (swap-chain fence, frame-latency waitable) *before* `NewFrame()`:
input is then sampled as late as possible, and the wait does not show up as UI time in `GetStats()`.

## Frame pacing

`wgt::FramePacer` (`wgt/pacing.hpp`) caps the frame rate for hosts that have no limiter of their own:

* it sleeps on a high-resolution waitable timer until shortly before the deadline, then spins the last
  ~1 ms;
* deadlines advance by a fixed period, so there is no drift, and the schedule re-anchors after a hitch.

Measured in the demo: 30 / 60 / 120 / 144 fps caps land at 33.33 / 16.67 / 8.33 / 6.94 ms. VSync stays
the swap chain's job. The demo hosts create tearing-capable swap chains, so "VSync off" is really
uncapped on displays that allow it.

## What is *not* thread-safe

* Widgets, Painter, `anim`, `ImGui::*` and font queries outside the UI thread.
* Keeping references returned by `GetTheme()` / `CurrentTheme()` beyond the current frame. Copy the
  `Theme` instead.
* Calling `Destroy()` while other threads still use the context. Stop your producers first.
