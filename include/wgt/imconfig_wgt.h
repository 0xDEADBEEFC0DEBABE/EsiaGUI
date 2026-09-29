// WGT UI - Dear ImGui configuration (selected through IMGUI_USER_CONFIG)
//
// Dear ImGui is compiled *inside* wgt.dll and exported from it, so the host application,
// wgt.dll and every plugin DLL share one ImGui implementation, one allocator and one context.
#pragma once

#define WGT_IMCONFIG_INCLUDED 1

#if defined(WGT_BUILD_DLL)
#   define IMGUI_API      __declspec(dllexport)
#   define IMGUI_IMPL_API
#elif defined(WGT_STATIC)
#   define IMGUI_API
#   define IMGUI_IMPL_API
#else
#   define IMGUI_API      __declspec(dllimport)
#   define IMGUI_IMPL_API
#endif

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_USE_WCHAR32               // full Unicode input (emoji and supplementary CJK are outside the BMP)
#define IMGUI_DISABLE_OBSOLETE_KEYIO
#define IMGUI_DEFINE_MATH_OPERATORS
// Text is WGT's (DirectWrite): no embedded ProggyClean font, glyphs never come from stb_truetype
// (a DirectWrite ImFontLoader is installed on the atlas before any font is added).
#define IMGUI_DISABLE_DEFAULT_FONT

// Thread-local current context.
// Every WGT context may live on its own thread; a stray ImGui call from a thread that does not own
// a context hits a null context (assert) instead of silently corrupting another thread's UI.
//  - inside wgt.dll GImGui is a real thread_local variable,
//  - outside (host / plugins) GImGui is routed through the exported ImGui::GetCurrentContext().
struct ImGuiContext;
#if defined(WGT_BUILD_DLL) || defined(WGT_STATIC)
extern thread_local ImGuiContext* WgtImGuiTLS;
#   define GImGui WgtImGuiTLS
#else
#   define GImGui (ImGui::GetCurrentContext())
#endif
