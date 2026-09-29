# Third-party code

## Dear ImGui (`imgui/`)

* Upstream: https://github.com/ocornut/imgui, version **1.92.9b** (commit `f1cc2ae`), MIT license (`imgui/LICENSE.txt`).
* WGT owns this copy and edits it in place (no `#ifdef` markers): `imgui.h`, `imgui.cpp`, `imgui_internal.h` and the
  Win32 platform backend carry WGT's changes, for example the `ItemSize` → `WgtOnItemLaidOut` hook that drives auto layout.
* `imgui-wgt.patch` is the full set of those edits against the upstream commit above. When updating ImGui, check out the
  new upstream version and re-apply it (`git apply --3way imgui-wgt.patch` inside `imgui/`).
