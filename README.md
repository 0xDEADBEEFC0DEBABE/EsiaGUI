# Esia for DirectX

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the DirectX backend alone: Direct3D 9,
10, 11 and 12, on Windows. A host that brings its own device uses that version (`esia/rhi/d3d11.hpp` ...); the
examples start the first version that works on the machine - 11, then 12, 10, 9 - and `--api d3d12` (or `d3d10`,
`d3d9`) picks one.

## Build

With Visual Studio 2022 and nothing else (or open the folder in Visual Studio and pick a preset):

```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

The examples are `build\windows-msvc\bin\Release\showcase.exe` and `glass_window.exe`. With LLVM instead (clang-cl
and lld on PATH, from an "x64 Native Tools" prompt): `cmake --preset windows-clang-cl`, then
`cmake --build --preset windows-clang-cl`. Keep the clone's path under about 140 characters: Visual Studio's build
fails on paths over 260.

## About this branch

It is generated from `main` (02015c6) by `tools/branches/backend_branches.py`: do not commit to it. `main` has every
backend (DirectX, OpenGL, Vulkan, Metal), the documents in `docs/`, the development history and the pull requests.
