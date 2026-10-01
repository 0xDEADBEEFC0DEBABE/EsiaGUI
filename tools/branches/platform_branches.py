#!/usr/bin/env python3
"""Platform branches: `windows`, `apple` and `linux` are `main` without what the platform does not build, so a user
clones one and builds it without choosing anything:

    git clone -b windows https://github.com/<owner>/EsiaGUI.git     # DirectX, OpenGL, Vulkan; the Win32 frame
    git clone -b apple   ...                                         # Metal; the AppKit and UIKit frames
    git clone -b linux   ...                                         # OpenGL, Vulkan; no example frame yet

Each branch gets its own README.md (two commands) and CMakePresets.json (its presets only). They are outputs: every
commit on them is made by this script from a commit of `main` (named in the message), on top of the branch's last
commit - nothing is rewritten or force-pushed. Development and pull requests go to `main`.

    python3 tools/branches/platform_branches.py [--source <commit>] [--remote origin] [--push] [--only windows ...]

Without --push it prints what each branch would get. The `Platform branches` workflow runs it after every push to
`main` (.github/workflows/branches.yml); it works the same from a local clone.
"""
import argparse
import json
import os
import subprocess
import sys
import tempfile

# What each branch leaves out of main (directories or files). Every CMake reference to these is behind a platform
# condition, so the rest configures unchanged; the examples directory is optional (top-level CMakeLists.txt).
COMMON_DROP = ['.github']
DROP = {
    'windows': [
        'src/esia/rhi/metal',
        'src/esia/text/system_fonts_apple.cpp', 'src/esia/text/system_fonts_unix.cpp',
        'examples/glass_window/Info-ios.plist.in', 'examples/glass_window/app_apple.hpp',
        'examples/glass_window/app_apple.mm', 'examples/glass_window/app_ios.mm', 'examples/glass_window/app_macos.mm',
        'examples/showcase/image_file_apple.mm',
        'tests/text/coretext_util.cpp',
        'cmake/toolchains/apple-ios.cmake', 'cmake/toolchains/clang-macos.cmake', 'cmake/toolchains/clang-linux.cmake',
        'tools/ios',
    ],
    'apple': [
        'src/esia/rhi/directx', 'src/esia/rhi/opengl', 'src/esia/rhi/vulkan',
        'src/esia/platform/win32', 'include/esia/platform',
        'src/esia/text/system_fonts_windows.cpp', 'src/esia/text/system_fonts_unix.cpp',
        'examples/glass_window/app.cpp', 'examples/glass_window/host.cpp', 'examples/glass_window/host.hpp',
        'examples/glass_window/host_d3d9.cpp', 'examples/glass_window/host_d3d10.cpp',
        'examples/glass_window/host_d3d11.cpp', 'examples/glass_window/host_d3d12.cpp',
        'examples/glass_window/host_opengl.cpp', 'examples/glass_window/host_vulkan.cpp',
        'examples/glass_window/dxgi_swap_chain.cpp', 'examples/glass_window/dxgi_swap_chain.hpp',
        'examples/glass_window/wine_screenshots',
        'examples/showcase/image_file.cpp',
        'cmake/toolchains/clang-cl-windows.cmake', 'cmake/toolchains/clang-cl-xwin.cmake',
        'cmake/toolchains/clang-linux.cmake', 'cmake/toolchains/clang-mingw.cmake',
    ],
    'linux': [
        'src/esia/rhi/directx', 'src/esia/rhi/metal',
        'src/esia/platform/win32', 'include/esia/platform',
        'src/esia/text/system_fonts_windows.cpp', 'src/esia/text/system_fonts_apple.cpp',
        'examples',
        'tests/text/coretext_util.cpp',
        'cmake/toolchains/apple-ios.cmake', 'cmake/toolchains/clang-macos.cmake',
        'cmake/toolchains/clang-cl-windows.cmake', 'cmake/toolchains/clang-cl-xwin.cmake',
        'cmake/toolchains/clang-mingw.cmake',
        'tools/ios',
    ],
}
# The configure presets each branch keeps (their build and test presets follow; hidden bases stay).
PRESETS = {
    'windows': ['windows-msvc', 'windows-clang-cl', 'windows-cross', 'windows-mingw-cross'],
    'apple': ['macos-clang', 'ios', 'ios-simulator'],
    'linux': ['linux-clang', 'linux-clang-release'],
}

README = {
    'windows': """# Esia for Windows

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with what Windows builds: DirectX
(Direct3D 9, 10, 11, 12), OpenGL, Vulkan (when the Vulkan SDK is installed) and the examples' Win32 frame.

## Build

With Visual Studio 2022 and nothing else (or open the folder in Visual Studio and pick a preset):

```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

Then run `build\\windows-msvc\\bin\\Release\\showcase.exe` or `glass_window.exe`. They start DirectX, the first
Direct3D version that works on the machine (11, then 12, 10, 9); `--api d3d12`, `--api opengl` or `--api vulkan`
picks another.

With LLVM instead (clang-cl and lld on PATH, from an "x64 Native Tools" prompt):

```
cmake --preset windows-clang-cl
cmake --build --preset windows-clang-cl
```
""",
    'apple': """# Esia for macOS and iOS

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with what Apple systems build: the Metal
backend and the examples' AppKit (macOS) and UIKit (iOS) frames.

## Build for macOS

macOS 11 or later, Xcode (the SDK and the Metal compiler) and Homebrew:

```
brew install llvm lld ninja cmake freetype harfbuzz
cmake --preset macos-clang
cmake --build --preset macos-clang
```

Then run `build/macos-clang/bin/showcase` or `glass_window`.

## Build for iOS

```
cmake --preset ios
cmake --build --preset ios
```

The examples become app bundles in `build/ios/bin`, signed when Xcode has a provisioning profile for them (sign in to
Xcode with your Apple ID); `xcrun devicectl device install app --device "<name>" build/ios/bin/showcase.app` puts one on
a connected iPhone. The simulator needs no signing: `cmake --preset ios-simulator`, `cmake --build --preset
ios-simulator`.
""",
    'linux': """# Esia for Linux

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with what Linux builds: the OpenGL /
OpenGL ES backend and Vulkan (when its headers are installed). There is no Linux example frame yet: the library, its
tests and the conformance suite.

## Build

clang, lld, Ninja and CMake (Ubuntu: `sudo apt install clang lld ninja-build cmake libvulkan-dev libfreetype-dev
libharfbuzz-dev libegl-dev`):

```
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```
""",
}
README_TAIL = """
Configuring prints the backends it builds (`-- Esia backends: ...`); `-DESIA_BACKEND_<NAME>=OFF` leaves one out.

This branch is generated from `main` ({source}) by `tools/branches/platform_branches.py`: do not commit to it. The
documents are in `docs/`; development, the other platforms and pull requests are on `main`.
"""


def git(*args, env=None, input=None):
    return subprocess.run(['git', *args], check=True, capture_output=True, text=True, env=env, input=input).stdout.strip()


def blob(text):
    return git('hash-object', '-w', '--stdin', input=text)


def presets(source, keep):
    p = json.loads(git('show', f'{source}:CMakePresets.json'))
    names = set(keep)
    configure = []
    for c in p['configurePresets']:   # keep the bases the kept presets inherit from
        if c['name'] in names or c.get('hidden'):
            configure.append(c)
    p['configurePresets'] = configure
    for key in ('buildPresets', 'testPresets'):
        if key in p:
            p[key] = [b for b in p[key] if b.get('configurePreset') in names]
    return json.dumps(p, indent=2) + '\n'


def tree_for(platform, source, short):
    with tempfile.TemporaryDirectory() as tmp:
        env = dict(os.environ, GIT_INDEX_FILE=os.path.join(tmp, 'index'))
        git('read-tree', source, env=env)
        for path in COMMON_DROP + DROP[platform]:
            git('rm', '--cached', '-r', '-q', '--ignore-unmatch', path, env=env)
        readme = README[platform] + README_TAIL.format(source=short)
        git('update-index', '--add', '--cacheinfo', f'100644,{blob(readme)},README.md', env=env)
        git('update-index', '--add', '--cacheinfo', f'100644,{blob(presets(source, PRESETS[platform]))},CMakePresets.json',
            env=env)
        return git('write-tree', env=env)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--source', default='HEAD', help='the commit of main to make the branches from (default HEAD)')
    ap.add_argument('--remote', default='origin')
    ap.add_argument('--push', action='store_true', help='commit and push the branches (default: only print)')
    ap.add_argument('--only', nargs='*', choices=sorted(DROP), help='these branches only')
    args = ap.parse_args()

    source = git('rev-parse', '--verify', args.source + '^{commit}')
    short = git('rev-parse', '--short', source)
    subject = git('log', '-1', '--format=%s', source)
    for platform in args.only or sorted(DROP):
        tree = tree_for(platform, source, short)
        try:
            parent = git('rev-parse', '--verify', '-q', f'refs/remotes/{args.remote}/{platform}')
        except subprocess.CalledProcessError:
            parent = ''
        if parent and git('rev-parse', parent + '^{tree}') == tree:
            print(f'{platform}: up to date ({parent[:9]})')
            continue
        message = f'{platform}: main {short} - {subject}\n\nGenerated from main by tools/branches/platform_branches.py.\n'
        if not args.push:
            files = git('ls-tree', '-r', '--name-only', tree).count('\n') + 1
            print(f'{platform}: would commit tree {tree[:9]} ({files} files) on {parent[:9] if parent else "a new branch"}')
            continue
        commit = git('commit-tree', tree, *(['-p', parent] if parent else []), '-m', message)
        git('push', args.remote, f'{commit}:refs/heads/{platform}')
        print(f'{platform}: {commit[:9]} pushed')
    return 0


if __name__ == '__main__':
    sys.exit(main())
