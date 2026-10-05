# Esia for Metal

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the Metal backend alone, on macOS and
iOS, and the examples' AppKit and UIKit frames.

## Build for macOS

macOS 11 or later, Xcode (the SDK and the Metal compiler) and Homebrew:

```
brew install llvm lld ninja cmake freetype harfbuzz
cmake --preset macos-clang
cmake --build --preset macos-clang
```

The examples are `build/macos-clang/bin/showcase` and `glass_window`.

## Build for iOS

```
cmake --preset ios
cmake --build --preset ios
```

The examples become app bundles in `build/ios/bin`, signed when Xcode has a provisioning profile for them (sign in to
Xcode with your Apple ID); `xcrun devicectl device install app --device "<name>" build/ios/bin/showcase.app` puts one on
a connected iPhone. The simulator needs no signing: `cmake --preset ios-simulator`, then
`cmake --build --preset ios-simulator`.

## About this branch

It is generated from `main` (e8c4ff4) by `tools/branches/backend_branches.py`: do not commit to it. `main` has every
backend (DirectX, OpenGL, Vulkan, Metal), the documents in `docs/`, the development history and the pull requests.

## License

MIT: [LICENSE](LICENSE).
