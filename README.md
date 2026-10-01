# Esia for Vulkan

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the Vulkan backend alone (Vulkan 1.1
and later), on Windows and Linux. It needs the Vulkan headers: on Windows the LunarG Vulkan SDK
(https://vulkan.lunarg.com; its installer sets `VULKAN_SDK`), on Linux the distribution's package. Without them
configuring says so and builds no backend.

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

clang, lld, Ninja and CMake (Ubuntu: `sudo apt install clang lld ninja-build cmake libfreetype-dev
libharfbuzz-dev libvulkan-dev`):

```
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```

There is no example window on Linux yet: the library, its tests and the conformance suite.

## About this branch

It is generated from `main` (4f78e61) by `tools/branches/backend_branches.py`: do not commit to it. `main` has every
backend (DirectX, OpenGL, Vulkan, Metal), the documents in `docs/`, the development history and the pull requests.
