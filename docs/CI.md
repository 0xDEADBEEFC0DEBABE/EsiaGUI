# Continuous integration

GitHub Actions runs three workflows (`.github/workflows`), one per host OS, on every push to `main` and every pull
request into it that changes more than documentation, and on any branch by hand (section 5: the minutes are
metered). A fourth, `Shaders`, only regenerates the shader library on `shaders/*` branches (section 5); a fifth,
`Platform branches`, commits `main` minus what each platform does not build to the branches `windows`, `apple` and
`linux` after every push to `main` (`tools/branches/platform_branches.py`, seconds on Linux). Every job
builds with `-DESIA_WERROR=ON`, runs `ctest`, writes a table of every test and every conformance scene to the job's
summary (`.github/scripts/ctest_report.py`) and uploads the conformance images, the diff images of failures, the
CTest JUnit file and the test logs as an artifact named after the job (kept 14 days).

A conformance test that finds no device for its backend exits with 77 and CTest shows it as *skipped*, not passed.
`ctest_report.py --require <regex>` turns a skipped test into a failure for the tests a job exists to run, so a
runner image that loses a driver cannot turn a job green by skipping everything.

* [1. Jobs](#1-jobs)
* [2. What CI proves and what it does not](#2-what-ci-proves-and-what-it-does-not)
* [3. Reproducing a job locally](#3-reproducing-a-job-locally)
* [4. Results](#4-results)
* [5. Maintenance](#5-maintenance)

## 1. Jobs

### Linux (`linux.yml`, ubuntu-24.04)

| Job | Runs | Required to run (not skip) |
| --- | --- | --- |
| `linux-clang`, `linux-clang-release` | clang 18 + lld; `ESIA_BACKEND_OPENGL`, `ESIA_BACKEND_VULKAN`, `ESIA_BACKEND_METAL` (its portable part); `ctest`: unit tests, the null backend's command-stream goldens, the OpenGL 3.3 / GLES 3.0 conformance suite on Mesa llvmpipe through EGL, the Vulkan conformance suite on lavapipe with the Khronos validation layer (synchronization validation on) on both render paths (dynamic rendering, `ESIA_VULKAN_RENDER_PASS=1`), each single frame and `--frames 3`; the GLSL / ESSL link test; the FreeType + HarfBuzz text goldens; Metal's unit tests on the fake GPU and its MSL / Objective-C++ mock checks | every OpenGL / GLES / Vulkan / null conformance test, the OpenGL, Vulkan and Metal unit tests, the GLSL link and text tests. Metal's conformance tests skip (no Metal on Linux) |
| `shaders` | `tools/shaders/build_shaders.py --check`: the checked-in SPIR-V / GLSL / ESSL / MSL library is what glslang + SPIRV-Cross produce from `src/esia/shaders` (Ubuntu 24.04's packages, the versions the library was generated with) | - |
| `windows-mingw-cross` | the `windows-mingw-cross` preset (clang + ld.lld + mingw-w64 13 posix) with every D3D backend plus OpenGL, Vulkan and Metal: a compile / link check of the Windows code with the GNU ABI; nothing runs | - |

The job pins Mesa's CPU drivers (`VK_DRIVER_FILES` = lavapipe's ICD, `LIBGL_ALWAYS_SOFTWARE`, `GALLIUM_DRIVER=llvmpipe`)
and fails before building when `vulkaninfo` shows no validation layer or no llvmpipe device; `logs/drivers.txt` in the
artifact has `vulkaninfo --summary` and `eglinfo -B`.

### Windows (`windows.yml`, windows-2022)

| Job | Runs |
| --- | --- |
| `windows-clang-cl` | the runner's LLVM clang-cl + lld-link (the `windows-clang-cl` preset, Ninja, the MSVC environment from `ilammy/msvc-dev-cmd`) |
| `windows-msvc` | MSVC 19.44 (the `windows-msvc` preset, Visual Studio 2022 generator, Debug) |

Both build every backend (`D3D9`, `D3D10`, `D3D11`, `D3D12`, `OPENGL`, `VULKAN`, `METAL`) and the text system, then
run `ctest` with:

* `ESIA_D3D_DRIVER=warp`: D3D10 / 11 / 12 on WARP (the runner has no GPU, only the Hyper-V video adapter);
* `ESIA_D3D_DEBUG=1`: the D3D10 / 11 / 12 debug layers (the runner image has the Graphics Tools optional feature:
  `d3d10sdklayers.dll`, `d3d11_3SDKLayers.dll`, `d3d12SDKLayers.dll`; the job installs it if a future image does not);
* OpenGL / GLES through Mesa for Windows ([pal1000/mesa-dist-win](https://github.com/pal1000/mesa-dist-win), pinned
  release `MESA_DIST_WIN_VERSION`): `opengl32.dll` and `libgallium_wgl.dll` copied next to the executables,
  `GALLIUM_DRIVER=llvmpipe`;
* FreeType + HarfBuzz from the runner's vcpkg (`x64-windows`) through vcpkg's toolchain file, which chain-loads the
  preset's (`VCPKG_CHAINLOAD_TOOLCHAIN_FILE`); a plain `CMAKE_PREFIX_PATH` does not work with vcpkg's harfbuzz config.

The job also installs what Vulkan needs - the LunarG SDK (headers, validation layer), the loader from the VulkanRT
components of the same version, and mesa-dist-win's lavapipe registered under
`HKLM\SOFTWARE\Khronos\Vulkan\Drivers` (the job runs elevated, and an elevated process's loader ignores
`VK_DRIVER_FILES`) - but see below.

**Not run** (`NOT_RUN_*` in `windows.yml`, excluded from `ctest -E`, listed in the summary; the Diagnostics step keeps
reproducing them with a stack):

| Tests | Why |
| --- | --- |
| `esia_conformance_d3d9`, `_frames` | The runner's Direct3D 9 device is WARP's D3D9 emulation (`Warp9UM` in `d3d10warp.dll` on the Hyper-V video adapter). Its ps_3_0 translator writes out of bounds translating the FX pixel shader: access violation in `d3d10warp!ShaderConv::CContext::EmitDstInstruction` from `Translate_TEXLDL`, under `IDirect3DDevice9::DrawIndexedPrimitive`, with full page heap on (without it: `0xC0000374` heap corruption or `0xC0000005`, every scene, with or without the debug layer). `esia_rhi_d3d9_tests` pass and still run. |
| `esia_rhi_vulkan_tests`, `esia_conformance_vulkan*` | mesa-dist-win 26.2.3's lavapipe corrupts its own heap on Windows: page-heap stop "corrupted start stamp" in `RtlFreeHeap` called from `vulkan_lvp.dll`'s `vkFreeMemory` (from `VulkanDevice::ReleaseTex`, a plain `vkDestroyImage` + `vkFreeMemory` without allocator callbacks) and from lavapipe's own queue thread. The same happens with the validation layer off (`ESIA_VULKAN_VALIDATION=0`), and the same tests are clean under AddressSanitizer with lavapipe on Linux. |

**Known failure** (`KNOWN_WARP` in `windows.yml`: listed with its numbers in the summary, a warning, not a job
failure; any other failure fails the job, and the entry is reported when it stops failing):

| Test | Scene | Numbers | Why |
| --- | --- | --- | --- |
| `esia_conformance_d3d12`, `_frames` | `glow_layer` | max delta 235, 0.081 - 0.082 % of the pixels over 10, mean delta 0.052 - 0.055 | see section 4 |

Required: the null, D3D10 / 11 / 12 and OpenGL / GLES conformance tests (single frame and `--frames 3`), every D3D
unit test (including D3D9's), the OpenGL unit tests and the text tests.

### macOS (`macos.yml`, macos-15, arm64)

| Job | Runs |
| --- | --- |
| `macos-clang` | Homebrew LLVM + lld (the `macos-clang` preset) against Xcode's SDK, `ESIA_BACKEND_METAL` and `ESIA_BACKEND_OPENGL`; `ctest` with `MTL_DEBUG_LAYER=1` and `MTL_SHADER_VALIDATION=1`: the Metal conformance suite on the runner's Metal device, the generated MSL compiled by `xcrun metal`, Metal's unit tests |
| `macos-clang (macOS 11.0)` | the same with `CMAKE_OSX_DEPLOYMENT_TARGET=11.0`, the backend's minimum: an API newer than macOS 11 used without `@available` is an error (`-Wunguarded-availability-new` with `ESIA_WERROR`) |

Required: the null and Metal conformance tests (single frame and `--frames 3`), Metal's unit tests and the MSL compile
test. OpenGL / GLES skip: the backend's headless devices are EGL and WGL, macOS has neither.

### Caches

Only fetched third-party inputs are cached, keyed by their version: the Vulkan SDK installer and runtime components
(`VULKAN_SDK_VERSION`) and vcpkg's binary packages of FreeType / HarfBuzz (keyed by the runner's vcpkg commit, so a
new image rebuilds them). No build directory, object file or test output is cached: every job configures and builds
from a clean checkout.

## 2. What CI proves and what it does not

**Proves**, on every run:

* every preset builds warning-free with `ESIA_WERROR` on clang 18 (Linux), the runner's clang-cl and MSVC 19.44
  (Windows), Homebrew LLVM against Apple's SDK (macOS), and the GNU-ABI cross build;
* the renderer, the core, the text system and every backend's logic pass their unit tests;
* the OpenGL, GLES and Vulkan backends render all conformance scenes within the goldens' tolerances on Mesa (Linux;
  OpenGL / GLES also on Mesa's Windows build through WGL), single frame and after poison frames, with zero GL errors
  and zero Vulkan validation / synchronization messages;
* the D3D10 / 11 / 12 backends render them on WARP with the debug layers counting messages (D3D12's `glow_layer`
  excepted, section 4), and every D3D backend's unit tests pass there;
* the Metal backend renders them on the macOS runner's Metal device with Metal's API and shader validation;
* the checked-in shader library matches its sources.

**Does not prove:**

* **Software renderers are not real drivers.** llvmpipe, lavapipe and WARP are conformant CPU implementations:
  they do not show a real driver's timing, memory limits, precision shortcuts, shader compiler bugs,
  tiling behaviour or synchronization races that a CPU executes in order anyway. A green job says the API is used
  correctly and the output is right on a reference implementation, not that NVIDIA / AMD / Intel / Apple / Qualcomm
  drivers agree. The goldens themselves come from llvmpipe.
* **The macOS runner's GPU is a virtual machine's** (see section 4 for what it reports): Metal on it is real Metal,
  but not the timing, the tile memory or the counter sampling of an Apple silicon GPU on bare metal.
* **Performance**: nothing is timed; CI machines are shared.
* **Presentation**: no swap chain, window, DPI or input path runs (the conformance suite is headless).
* **Direct3D 9 and Vulkan do not render on Windows** in CI (section 1: both crash inside the software driver the
  runner offers); Vulkan's Windows code is only compiled there. D3D12's GPU-based validation (`ESIA_D3D_DEBUG=2`) is
  not run.
* **The mingw-w64 build** only compiles and links; the shipped Windows ABI is MSVC's (the Windows jobs).
* **iOS** is not built.

## 3. Reproducing a job locally

Linux (Ubuntu 24.04, as the runner):

```bash
sudo apt-get install -y clang lld llvm cmake ninja-build python3 glslang-tools spirv-cross spirv-tools \
    libegl-dev libgles-dev libgl-dev libegl-mesa0 libgl1-mesa-dri mesa-utils \
    libvulkan-dev libvulkan1 mesa-vulkan-drivers vulkan-validationlayers vulkan-tools libfreetype-dev libharfbuzz-dev
export VK_DRIVER_FILES=/usr/share/vulkan/icd.d/lvp_icd.json LIBGL_ALWAYS_SOFTWARE=1 GALLIUM_DRIVER=llvmpipe
cmake --preset linux-clang -DESIA_WERROR=ON -DESIA_BACKEND_OPENGL=ON -DESIA_BACKEND_VULKAN=ON -DESIA_BACKEND_METAL=ON
cmake --build --preset linux-clang
ctest --preset linux-clang --output-junit ctest-junit.xml --test-output-size-passed 65536
python3 .github/scripts/ctest_report.py build/linux-clang/ctest-junit.xml \
    --require 'esia_conformance_(null|opengl|gles|vulkan)(_frames)?'
python3 tools/shaders/build_shaders.py --check                    # the shaders job

# windows-mingw-cross
sudo apt-get install -y g++-mingw-w64-x86-64-posix mingw-w64-x86-64-dev
mkdir -p /tmp/vkh && ln -sf /usr/include/vulkan /usr/include/vk_video /tmp/vkh/
cmake --preset windows-mingw-cross -DESIA_WERROR=ON -DESIA_BACKEND_D3D9=ON -DESIA_BACKEND_D3D10=ON \
    -DESIA_BACKEND_D3D11=ON -DESIA_BACKEND_D3D12=ON -DESIA_BACKEND_OPENGL=ON -DESIA_BACKEND_VULKAN=ON \
    -DESIA_BACKEND_METAL=ON -DESIA_VULKAN_INCLUDE_DIR=/tmp/vkh
cmake --build --preset windows-mingw-cross
```

(`ESIA_VULKAN_INCLUDE_DIR` must hold the Vulkan headers alone: `-I/usr/include` would put glibc's headers in front of
mingw-w64's.)

Windows, from an "x64 Native Tools Command Prompt for VS 2022" with LLVM first on `PATH`:

```bat
vcpkg install freetype harfbuzz --triplet x64-windows
set ESIA_D3D_DRIVER=warp
set ESIA_D3D_DEBUG=1
cmake --preset windows-clang-cl -DESIA_WERROR=ON -DESIA_BACKEND_D3D9=ON -DESIA_BACKEND_D3D10=ON ^
    -DESIA_BACKEND_D3D11=ON -DESIA_BACKEND_D3D12=ON -DESIA_BACKEND_OPENGL=ON -DESIA_BACKEND_VULKAN=ON ^
    -DESIA_BACKEND_METAL=ON -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake ^
    -DVCPKG_TARGET_TRIPLET=x64-windows -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%CD%/cmake/toolchains/clang-cl-windows.cmake
cmake --build --preset windows-clang-cl
ctest --preset windows-clang-cl
```

Leave `ESIA_D3D_DRIVER` unset to test the machine's GPU instead of WARP. To reproduce the Mesa part, put
mesa-dist-win's `x64\opengl32.dll` and `libgallium_wgl.dll` into `build\windows-clang-cl\bin` and set
`VK_DRIVER_FILES` to its `x64\lvp_icd.x86_64.json`; without them OpenGL and Vulkan use the machine's drivers.
MSVC: `cmake --preset windows-msvc ...` (the same options without the chain-load), then
`cmake --build --preset windows-msvc-debug` and `ctest --preset windows-msvc`.

macOS (Xcode installed):

```bash
brew install llvm lld ninja freetype harfbuzz
cmake --preset macos-clang -DESIA_WERROR=ON -DESIA_BACKEND_METAL=ON -DESIA_BACKEND_OPENGL=ON   # + -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build --preset macos-clang
MTL_DEBUG_LAYER=1 MTL_SHADER_VALIDATION=1 ctest --preset macos-clang
```

## 4. Results

Seen in this session through the GitHub API (runs on `feat/ci`, 2026-09-29); the tables are from the job summaries.
**All three workflows are green on `94865f4`** (Linux run 9, macOS run 8, Windows run 8), with the Windows exceptions
listed in section 1. The two commits after it (the Diagnostics step's timeout, these documents) were pushed later:
check their runs on the branch.

### Linux (ubuntu-24.04: clang 18.1.3, lld 18.1.3, Mesa 25.2.8 with LLVM 20.1.2, validation layer 1.3.275)

All four jobs green. `linux-clang` and `linux-clang-release`: 23 tests, 21 passed, 2 skipped (Metal's conformance
tests: no Metal on Linux). All 17 scenes pass on OpenGL, GLES, Vulkan (dynamic rendering) and Vulkan
(render passes), single frame and `--frames 3`, with zero GL errors and zero validation messages. Worst scene of the
eight runs: max delta 4 (`hidpi`, `srgb_target`, `srgb_msaa`), mean delta at most 0.001, 0.000 % of the pixels over
tolerance. `shaders`: the library is up to date. `windows-mingw-cross`: builds after one fix (below).

### macOS (macos-15 arm64: macOS 15.7.9, Xcode 16.4, Homebrew clang 23.1.0, Apple Paravirtual device)

Both jobs green, 16 tests: 12 passed, 4 skipped (OpenGL / GLES: no EGL on macOS). **The first real Metal run**:
`metal_device.mm` compiled against the macOS SDK without a change, also at `CMAKE_OSX_DEPLOYMENT_TARGET=11.0`; `xcrun
metal` compiled every generated MSL file; the conformance suite passed every scene with `MTL_DEBUG_LAYER=1` and
`MTL_SHADER_VALIDATION=1`, identical single frame and `--frames 3`:

| Scene | max delta | pixels over | mean delta |
| --- | --- | --- | --- |
| shapes, fx_rows | 1 | 0.000 % | 0.023 |
| gradients | 2 | 0.000 % | 0.010 |
| shadows | 20 | 0.003 % | 0.051 |
| glass, msaa_target, glass_copy, callback_capture | 6 | 0.000 % | 0.046 |
| glow_layer | 1 | 0.000 % | 0.019 |
| text | 2 | 0.000 % | 0.004 |
| edge_fade, clipping | 2 | 0.000 % | 0.020 |
| windows | 4 | 0.000 % | 0.045 |
| hidpi | 3 | 0.000 % | 0.035 |
| light_streak | 4 | 0.000 % | 0.030 |
| srgb_target, srgb_msaa | 6 | 0.000 % | 0.091 |

The device (`MTLCreateSystemDefaultDevice`): "Apple Paravirtual device", GPU family Mac2, not Apple7, no counter
sampling at stage boundaries - so `timestampQueries` is false and the profiler's Metal path did not run. Details in
`src/esia/rhi/metal/STATUS.md`.

### Windows (windows-2022: Windows Server 2022 20348, clang-cl 20.1.8, MSVC 19.44, Microsoft Hyper-V Video)

Both compilers build every backend with `ESIA_WERROR=ON` without a source change; their results are the same (the
llvmpipe sRGB scenes' mean delta is 0.015 with clang-cl, 0.021 with MSVC).

| Backend (device) | Result |
| --- | --- |
| D3D10, D3D11 (WARP, debug layer) | 17 / 17, single frame and `--frames 3` |
| D3D12 (WARP, debug layer) | 16 / 17: `glow_layer` fails (below) |
| D3D9 (WARP's D3D9 emulation) | not run: crashes inside `d3d10warp.dll` (section 1); unit tests pass |
| OpenGL, GLES (mesa-dist-win 26.2.3 llvmpipe through WGL) | 17 / 17, single frame and `--frames 3` |
| Vulkan (mesa-dist-win 26.2.3 lavapipe, SDK 1.3.296.0) | not run: lavapipe corrupts its heap (section 1) |

Numbers against the llvmpipe goldens (max delta / mean delta; 0.000 % of the pixels over tolerance unless noted),
identical for D3D10, D3D11 and D3D12 except `glow_layer`:

| Scene | OpenGL / GLES (llvmpipe, WGL) | D3D10 / 11 / 12 (WARP) |
| --- | --- | --- |
| shapes, fx_rows | 0 / 0.000 | 1 / 0.002 |
| gradients | 0 / 0.000 | 2 / 0.008 |
| shadows | 1 / 0.001 | 20 / 0.002 (0.001 % over) |
| glass, msaa_target, glass_copy, callback_capture | 2 / 0.016 | 3 / 0.028 |
| glow_layer | 2 / 0.000 | D3D10 / 11: 1 / 0.009; **D3D12: 235 / 0.052 - 0.055, 0.081 - 0.082 % over - FAIL** |
| text | 2 / 0.001 | 2 / 0.004 |
| edge_fade | 2 / 0.002 | 1 / 0.003 |
| clipping | 2 / 0.003 | 1 / 0.017 |
| windows | 1 / 0.016 | 4 / 0.018 |
| hidpi | 1 / 0.015 | 3 / 0.016 |
| light_streak | 3 / 0.001 | 4 / 0.018 |
| srgb_target, srgb_msaa | 2 / 0.015 | 4 / 0.091 |

**WARP deltas.** WARP stays within max delta 4 and mean 0.1 of the llvmpipe goldens everywhere except `shadows`
(max 20 on 0.001 % of the pixels, the same outlier NVIDIA and Metal show) and D3D12's `glow_layer`. That failure is
a single column: x = 133, y = 42 - 111 (62 pixels), where the glow layer holds pink glow from elsewhere in the scene
(up to 225, 96, 142 over the golden's 18, 47, 57) - with and without the debug layer, single frame and after poison
frames, in every run (its mean moves by a few thousandths between runs: WARP12 is not bit-exact run to run here).
D3D10 and D3D11 on the same WARP render the scene within 1, and D3D12 on an RTX 4080 passes
(`src/esia/rhi/directx/common/STATUS.md`), so it is WARP12 together with the D3D12 backend's glow path (region clears,
bloom pyramid), not a tolerance question: the tolerances are unchanged, the scene is recorded as a known failure and
left to the D3D12 backend. `logs/`, the diff image and the rendered image are in the job's artifact (and as base64 in
the Diagnostics step's log).

### Fixes the new platforms forced

* `windows-mingw-cross` did not link on `main`: `d3d_util.hpp`'s `Logger` used `std::make_shared`, which instantiates
  `std::type_info::operator==` twice with clang + mingw-w64's libstdc++ 13 (`docs/backends/README.md`, section 6).
  Fixed in `c90865d`.
* The Windows configure failed with vcpkg's FreeType + HarfBuzz: FindFreetype defined `Freetype::Freetype`, then
  harfbuzz's config loaded freetype's config, which refuses to load after it. `src/esia/CMakeLists.txt` now looks for
  HarfBuzz first (`8aaae61`).
* None in `src/esia/rhi/metal`: the Metal code compiled as written.

### Found, not fixed here

* **AddressSanitizer** (run once in this session on Linux, not a CI job): `GlDevice.HostStateIsRestored`
  (`src/esia/rhi/opengl/tests/test_gl_device.cpp:418`) uploads a 4-byte `white[4]` into an `RGBA16_FLOAT` texture
  (8 bytes per pixel): an 8-byte read from a 4-byte stack array in `GlDevice::Upload`. A test bug; the rest of the
  ASan run (core, renderer, text, OpenGL / GLES / Vulkan conformance, Vulkan unit tests) was clean. An ASan job would
  be a cheap addition once it is fixed.
* The D3D12 `glow_layer` column on WARP and the two Windows driver crashes above.

## 5. Maintenance

* **Minutes**: the repository is private, so the hosted runners' minutes come out of the account's monthly
  allowance (3,000 with GitHub Pro), macOS at 10x and Windows at 2x: one run of the three workflows is about
  100 minutes, most of it macOS. Hence the triggers: pushes to `main` and pull requests into it, not feature branches,
  and no run for a change to `docs/` or Markdown alone; `[skip ci]` in the head commit's message skips a push.
  Branches are tested locally first (every backend on Windows with real GPUs); to check a branch on another OS
  before merging, run only the workflow that matters from the Actions tab (*Run workflow*, pick the branch). Job
  timeouts are about three times a normal run (Linux 30, macOS 20, Windows 45 minutes), so a hang costs little.
  Usage: *Settings > Billing and licensing > Overview*.
* **Goldens**: a rendering change that moves a scene past its tolerance fails every backend's job. Regenerate with
  OpenGL on llvmpipe (`docs/backends/README.md`, section 7) after reviewing the uploaded `*.diff.png` files; never
  widen a tolerance for one backend's software renderer. Where llvmpipe is not at hand, carry the change into the
  goldens instead: golden + (new - old), per pixel, from two renders of one backend on one device with and without
  the change (`esia_conformance --out`), so every pixel the change does not touch stays llvmpipe's (the glass rim
  filter, 2026-10-01, from D3D11).
* **Shader library**: the generated files must be exactly what Ubuntu 24.04's glslang and SPIRV-Cross make (the
  `shaders` check), and other versions (a Vulkan SDK's) make different files. The files themselves run anywhere
  (SPIR-V, GLSL, ESSL and MSL do not depend on where they were made); the versions are pinned only so the check can
  regenerate them byte for byte. Where those packages are not
  installed, push the shader change to a branch named `shaders/<anything>`: the `Shaders` workflow
  (`shaders.yml`, about a minute on Linux) regenerates the library and commits it to that branch. Take its
  `src/esia/shaders/generated` into the change, then delete the branch.
* **Pinned versions**: `VULKAN_SDK_VERSION` and `MESA_DIST_WIN_VERSION` in `windows.yml`. Updating one changes what
  the Windows tests run on: note the new numbers here.
* **Runner images** move (clang, Mesa, Xcode versions). The toolchain and driver versions of each run are at the top
  of its log and in `logs/` in the artifact.
