// WGT UI - umbrella header
//
//   #include <wgt/wgt.hpp>
//
//   auto* ui = wgt::Context::Create({ .backend = wgt::Backend::D3D11, .hwnd = hwnd,
//                                     .d3d11Device = device, .d3d11Context = immediateContext });
//   // WndProc:   if (ui->HandleWin32Message(hwnd, msg, wParam, lParam)) return 0;
//   // Frame:
//   ui->NewFrame();
//   if (wgt::ui::BeginWindow("Hello", &open)) { wgt::ui::Button("Glass!", {.kind = wgt::ui::ButtonKind::Glass}); wgt::ui::EndWindow(); }
//   ui->Render(wgt::RenderTarget::D3D11(backBufferRtv));
#pragma once
#include "config.hpp"
#include "math.hpp"
#include "function.hpp"
#include "sync.hpp"
#include "theme.hpp"
#include "anim.hpp"
#include "fx.hpp"
#include "painter.hpp"
#include "context.hpp"
#include "pacing.hpp"
#include "backend.hpp"
#include "icons.hpp"
#include "ui.hpp"
