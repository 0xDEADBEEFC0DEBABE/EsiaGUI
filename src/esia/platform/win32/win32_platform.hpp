// Esia - Win32 platform layer: the state behind esia::platform::win32::Platform (private to the target).
#pragma once
#include "esia/platform/win32.hpp"
#include <windows.h>
#include <imm.h>
#include <atomic>
#include <bitset>
#include <mutex>
#include <string>
#include <string_view>

namespace esia::platform::win32
{
    std::wstring Widen(std::string_view utf8);
    std::string Narrow(const wchar_t* utf16, std::size_t count);

    struct Platform::Impl
    {
        explicit Impl(const Desc& d);

        Desc desc;
        // Registered, not WM_APP based: a host window may use any WM_APP value itself.
        const UINT updateMessage;    // posted by ApplyRequests: apply the pending requests on the window thread
        const UINT destroyMessage;   // posted by DestroyAppWindow

        // ---- shared between the threads
        std::atomic<HWND> hwnd{nullptr};
        std::atomic<float> uiScale;
        std::atomic<bool> wantMouse{false}, wantKeyboard{false}, wantText{false}, closeRequested{false};

        std::mutex contextMutex;             // held while queueing: SetContext(nullptr) waits for the window thread
        Context* context = nullptr;

        mutable std::mutex frameMutex;       // written by the window thread, read by Frame()
        FrameInfo frame;

        std::mutex requestMutex;             // ApplyRequests -> window thread
        PlatformRequests requests;
        bool updatePosted = false;

        // ---- window thread
        bool owned = false;                  // made by CreateAppWindow
        UINT dpi = USER_DEFAULT_SCREEN_DPI;
        std::uint32_t buttonsDown = 0;       // bit per MouseButton pressed in the window
        bool capturedMouse = false;          // SetCapture was ours
        bool trackingLeave = false, mouseInside = false;
        Vec2 lastMousePos{-1, -1};
        std::bitset<(std::size_t)Key::Count> keysDown;
        wchar_t highSurrogate = 0;
        bool composing = false;
        // requests as applied to the window
        MouseCursor cursor = MouseCursor::Arrow;
        bool textInput = false;
        Rect imeRect;
        int caretHeight = 0;                 // of the hidden system caret, 0 = none
        HCURSOR cursors[8] = {};

        void Queue(InputEvent e);
        float Scale() const { return (float)dpi / (float)USER_DEFAULT_SCREEN_DPI * uiScale.load(); }
        void Publish(int width, int height, bool minimized);
        void PublishClientRect(HWND w);
        void PublishFocus(bool focused);

        // input
        void OnMouseMove(LPARAM lparam);
        void OnMouseButton(HWND w, MouseButton b, bool down, LPARAM lparam);
        void ReleaseButtons();
        void LeaveIfOutside(HWND w);
        void OnKey(bool down, WPARAM wparam, LPARAM lparam);
        void OnChar(wchar_t c);
        void LoseFocus(HWND w);

        // requests, IME
        bool OwnsCursor() const;
        void ApplyPending(HWND w);
        void SetTextInput(HWND w, bool on);
        RECT CaretPixels() const;
        void PlaceIme(HWND w);
        void ReadComposition(HWND w, LPARAM flags);
        void EndComposition();
    };
}
