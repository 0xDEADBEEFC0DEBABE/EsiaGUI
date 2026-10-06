#!/usr/bin/env python3
"""Backend branches: `directx`, `opengl`, `vulkan` and `metal` are `main` with one backend, so a user clones the
branch of the graphics API they use and builds it without choosing anything:

    git clone -b directx https://github.com/<owner>/EsiaGUI.git     # Windows: Direct3D 11, 12, 10 or 9, the first that works
    git clone -b opengl  ...                                         # Windows (WGL), Linux, Android (EGL)
    git clone -b vulkan  ...                                         # Windows, Linux, Android (the Vulkan headers)
    git clone -b metal   ...                                         # macOS, iOS

Each branch has the core, the renderer, the widgets, its backend, the example frames of the systems that backend runs
on with that backend's hosts only, its own README.md (the commands that build it) and CMakePresets.json (those
systems' presets). They are outputs: every commit on them is made by this script from a commit of `main` (named in the
message), on top of the branch's last commit - nothing is rewritten or force-pushed. Development and pull requests go
to `main`, which has every backend.

    python3 tools/branches/backend_branches.py [--source <commit>] [--remote origin] [--push] [--only directx ...]

Without --push it prints what each branch would get. The `Backend branches` workflow runs it after every push to
`main` (.github/workflows/branches.yml); it works the same from a local clone.
"""
import argparse
import json
import os
import subprocess
import sys
import tempfile

BACKENDS = ['src/esia/rhi/directx', 'src/esia/rhi/opengl', 'src/esia/rhi/vulkan', 'src/esia/rhi/metal']
# The example hosts of each API (glass_window builds the ones whose backend is there): OpenGL has one per system (WGL,
# EGL on X11 and Android), the Vulkan host serves all three.
HOSTS = {
    'directx': ['examples/glass_window/host_d3d9.cpp', 'examples/glass_window/host_d3d10.cpp',
                'examples/glass_window/host_d3d11.cpp', 'examples/glass_window/host_d3d12.cpp',
                'examples/glass_window/dxgi_swap_chain.cpp', 'examples/glass_window/dxgi_swap_chain.hpp'],
    'opengl': ['examples/glass_window/host_opengl.cpp', 'examples/glass_window/host_opengl_egl.cpp'],
    'vulkan': ['examples/glass_window/host_vulkan.cpp'],
}
# The hosts' interface (Windows and Linux); the icon text system of the frames without Windows' icon fonts (Apple, Linux).
HOST_COMMON = ['examples/glass_window/host.cpp', 'examples/glass_window/host.hpp']
SYMBOLS = ['examples/glass_window/symbol_text.hpp']
# What only Apple systems build; what only Windows builds; what only Linux builds.
APPLE = ['src/esia/text/system_fonts_apple.cpp', 'examples/glass_window/Info-ios.plist.in',
         'examples/glass_window/app_apple.hpp', 'examples/glass_window/app_apple.mm', 'examples/glass_window/app_ios.mm',
         'examples/glass_window/app_macos.mm', 'examples/showcase/image_file_apple.mm', 'tests/text/coretext_util.cpp',
         'cmake/toolchains/apple-ios.cmake', 'cmake/toolchains/clang-macos.cmake', 'tools/ios']
WINDOWS = ['src/esia/platform/win32', 'include/esia/platform', 'src/esia/text/system_fonts_windows.cpp',
           'examples/glass_window/app.cpp', 'examples/showcase/image_file.cpp', 'cmake/toolchains/clang-cl-windows.cmake',
           'cmake/toolchains/clang-cl-xwin.cmake', 'cmake/toolchains/clang-mingw.cmake']
LINUX = ['src/esia/text/system_fonts_unix.cpp', 'cmake/toolchains/clang-linux.cmake', 'src/esia/platform/x11',
         'include/esia/platform/x11.hpp', 'examples/glass_window/app_linux.cpp',
         'examples/glass_window/icons_linux.cpp', 'examples/glass_window/icons_linux.hpp',
         'examples/showcase/image_file_linux.cpp']
# What only Android builds (with OpenGL ES or Vulkan: the opengl and vulkan branches).
ANDROID = ['src/esia/platform/android', 'include/esia/platform/android.hpp', 'examples/glass_window/app_android.cpp',
           'examples/glass_window/icon_font_text.hpp', 'examples/glass_window/icons_material.cpp',
           'examples/glass_window/icons_material.hpp', 'tools/android']
# Every branch: no CI of its own (main's covers it), no screenshots of other APIs.
COMMON_DROP = ['.github', 'examples/glass_window/wine_screenshots']


def others(keep, table):
    return [p for name, paths in table.items() if name != keep for p in paths]


def drop(branch):
    backend = 'src/esia/rhi/' + branch
    paths = [b for b in BACKENDS if b != backend]
    if branch == 'directx':
        return paths + others('directx', HOSTS) + APPLE + LINUX + ANDROID + SYMBOLS
    if branch in ('opengl', 'vulkan'):
        return paths + others(branch, HOSTS) + APPLE
    if branch == 'metal':
        return paths + [p for h in HOSTS.values() for p in h] + HOST_COMMON + WINDOWS + LINUX + ANDROID
    raise ValueError(branch)


BRANCHES = ['directx', 'opengl', 'vulkan', 'metal']
# The configure presets each branch keeps (their build and test presets follow; hidden bases stay).
WINDOWS_PRESETS = ['windows-msvc', 'windows-clang-cl', 'windows-cross', 'windows-mingw-cross']
LINUX_PRESETS = ['linux-clang', 'linux-clang-release']
PRESETS = {
    'directx': WINDOWS_PRESETS,
    'opengl': WINDOWS_PRESETS + LINUX_PRESETS + ['android'],
    'vulkan': WINDOWS_PRESETS + LINUX_PRESETS + ['android'],
    'metal': ['macos-clang', 'ios', 'ios-simulator'],
}

WINDOWS_BUILD = """With Visual Studio 2022 and nothing else (or open the folder in Visual Studio and pick a preset):

```
cmake --preset windows-msvc
cmake --build --preset windows-msvc-release
```

The examples are `build\\windows-msvc\\bin\\Release\\showcase.exe`, `workbench.exe` and `glass_window.exe`. With LLVM instead (clang-cl
and lld on PATH, from an "x64 Native Tools" prompt): `cmake --preset windows-clang-cl`, then
`cmake --build --preset windows-clang-cl`. Keep the clone's path under about 140 characters: Visual Studio's build
fails on paths over 260.
"""
LINUX_BUILD = """clang, lld, Ninja and CMake, and X11 for the examples (Ubuntu: `sudo apt install clang lld ninja-build cmake
libfreetype-dev libharfbuzz-dev libfontconfig-dev libx11-dev libjpeg-dev{extra}`):

```
cmake --preset linux-clang
cmake --build --preset linux-clang && ctest --preset linux-clang
```

The examples are `build/linux-clang/bin/showcase`, `workbench` and `glass_window`: an X11 window (XWayland on a Wayland desktop),
the desktop's icon theme for the icons, Ubuntu's wallpaper.
"""
ANDROID_BUILD = """The Android NDK and SDK (a platform, build-tools and platform-tools; a JDK for the APK signing tools), CMake and
Ninja:

```
export ANDROID_NDK_HOME=<the NDK> ANDROID_HOME=<the SDK>
cmake --preset android
cmake --build --preset android
adb install build/android/apk/showcase.apk
```

The examples become APKs in `build/android/apk`, for arm64 phones (`-DANDROID_ABI=x86_64` for the emulator). Their
icons are Google's Material Icons, downloaded when configuring.
"""
README = {
    'directx': """# Esia for DirectX

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the DirectX backend alone: Direct3D 9,
10, 11 and 12, on Windows. A host that brings its own device uses that version (`esia/rhi/d3d11.hpp` ...); the
examples start the first version that works on the machine - 11, then 12, 10, 9 - and `--api d3d12` (or `d3d10`,
`d3d9`) picks one.

## Build

""" + WINDOWS_BUILD,
    'opengl': """# Esia for OpenGL

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the OpenGL backend alone: OpenGL 3.3
core and OpenGL ES 3.0, on Windows (WGL), Linux and Android (EGL).

## Build on Windows

""" + WINDOWS_BUILD + """
## Build on Linux

""" + LINUX_BUILD.format(extra=' libegl-dev') + """
## Build for Android

""" + ANDROID_BUILD,
    'vulkan': """# Esia for Vulkan

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the Vulkan backend alone (Vulkan 1.1
and later), on Windows, Linux and Android. It needs the Vulkan headers: on Windows the LunarG Vulkan SDK
(https://vulkan.lunarg.com; its installer sets `VULKAN_SDK`), on Linux the distribution's package; the Android NDK has
them. Without them configuring says so and builds no backend.

## Build on Windows

""" + WINDOWS_BUILD + """
## Build on Linux

""" + LINUX_BUILD.format(extra=' libvulkan-dev') + """
## Build for Android

""" + ANDROID_BUILD,
    'metal': """# Esia for Metal

A liquid-glass UI toolkit for games and tools (C++20). This branch is `main` with the Metal backend alone, on macOS and
iOS, and the examples' AppKit and UIKit frames.

## Build for macOS

macOS 11 or later, Xcode (the SDK and the Metal compiler) and Homebrew:

```
brew install llvm lld ninja cmake freetype harfbuzz
cmake --preset macos-clang
cmake --build --preset macos-clang
```

The examples are `build/macos-clang/bin/showcase`, `workbench` and `glass_window`.

## Build for iOS

```
cmake --preset ios
cmake --build --preset ios
```

The examples become app bundles in `build/ios/bin`, signed when Xcode has a provisioning profile for them (sign in to
Xcode with your Apple ID); `xcrun devicectl device install app --device "<name>" build/ios/bin/showcase.app` puts one on
a connected iPhone. The simulator needs no signing: `cmake --preset ios-simulator`, then
`cmake --build --preset ios-simulator`.
""",
}
README_TAIL = """
## About this branch

It is generated from `main` ({source}) by `tools/branches/backend_branches.py`: do not commit to it. `main` has every
backend (DirectX, OpenGL, Vulkan, Metal), the documents in `docs/`, the development history and the pull requests.

## License

MIT: [LICENSE](LICENSE).
"""


def git(*args, env=None, input=None):
    return subprocess.run(['git', *args], check=True, capture_output=True, text=True, env=env, input=input).stdout.strip()


def blob(text):
    # bytes, not text: on Windows a text pipe would write CRLF line ends, and the branch would differ from CI's
    return subprocess.run(['git', 'hash-object', '-w', '--stdin'], check=True, capture_output=True,
                          input=text.encode('utf-8')).stdout.decode().strip()


def presets(source, keep):
    p = json.loads(git('show', f'{source}:CMakePresets.json'))
    names = set(keep)
    p['configurePresets'] = [c for c in p['configurePresets'] if c['name'] in names or c.get('hidden')]
    for key in ('buildPresets', 'testPresets'):
        if key in p:
            p[key] = [b for b in p[key] if b.get('configurePreset') in names]
    return json.dumps(p, indent=2) + '\n'


def tree_for(branch, source, short):
    with tempfile.TemporaryDirectory() as tmp:
        env = dict(os.environ, GIT_INDEX_FILE=os.path.join(tmp, 'index'))
        git('read-tree', source, env=env)
        for path in COMMON_DROP + drop(branch):
            git('rm', '--cached', '-r', '-q', '--ignore-unmatch', path, env=env)
        readme = README[branch] + README_TAIL.format(source=short)
        git('update-index', '--add', '--cacheinfo', f'100644,{blob(readme)},README.md', env=env)
        git('update-index', '--add', '--cacheinfo', f'100644,{blob(presets(source, PRESETS[branch]))},CMakePresets.json',
            env=env)
        return git('write-tree', env=env)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    ap.add_argument('--source', default='HEAD', help='the commit of main to make the branches from (default HEAD)')
    ap.add_argument('--remote', default='origin')
    ap.add_argument('--push', action='store_true', help='commit and push the branches (default: only print)')
    ap.add_argument('--only', nargs='*', choices=BRANCHES, help='these branches only')
    args = ap.parse_args()

    source = git('rev-parse', '--verify', args.source + '^{commit}')
    short = git('rev-parse', '--short', source)
    subject = git('log', '-1', '--format=%s', source)
    for branch in args.only or BRANCHES:
        tree = tree_for(branch, source, short)
        try:
            parent = git('rev-parse', '--verify', '-q', f'refs/remotes/{args.remote}/{branch}')
        except subprocess.CalledProcessError:
            parent = ''
        if parent and git('rev-parse', parent + '^{tree}') == tree:
            print(f'{branch}: up to date ({parent[:9]})')
            continue
        if not args.push:
            files = git('ls-tree', '-r', '--name-only', tree).count('\n') + 1
            print(f'{branch}: would commit tree {tree[:9]} ({files} files) on {parent[:9] if parent else "a new branch"}')
            continue
        message = f'{branch}: main {short} - {subject}\n\nGenerated from main by tools/branches/backend_branches.py.\n'
        commit = git('commit-tree', tree, *(['-p', parent] if parent else []), '-m', message)
        git('push', args.remote, f'{commit}:refs/heads/{branch}')
        print(f'{branch}: {commit[:9]} pushed')
    return 0


if __name__ == '__main__':
    sys.exit(main())
