# Esia for OpenGL

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the OpenGL backend alone: OpenGL 3.3
core and OpenGL ES 3.0, on Windows (WGL) and Linux (EGL).

## Build on Windows

With Visual Studio 2022 and nothing else (or open the folder in Visual Studio and pick a preset):

```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

The examples are `build\windows-msvc\bin\Release\showcase.exe` and `glass_window.exe`. With LLVM instead (clang-cl
and lld on PATH, from an "x64 Native Tools" prompt): `cmake --preset windows-clang-cl`, then
`cmake --build --preset windows-clang-cl`. Keep the clone's path under about 140 characters: Visual Studio's build
fails on paths over 260.

## Build on Linux

clang, lld, Ninja and CMake, and X11 for the examples (Ubuntu: `sudo apt install clang lld ninja-build cmake
libfreetype-dev libharfbuzz-dev libx11-dev libjpeg-dev libegl-dev`):

```
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```

The examples are `build/linux-clang/bin/showcase` and `glass_window`: an X11 window (XWayland on a Wayland desktop),
the desktop's icon theme for the icons, Ubuntu's wallpaper.

## About this branch

It is generated from `main` (82fb3aa) by `tools/branches/backend_branches.py`: do not commit to it. `main` has every
backend (DirectX, OpenGL, Vulkan, Metal), the documents in `docs/`, the development history and the pull requests.
