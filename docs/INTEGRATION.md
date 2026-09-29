# WGT 对接文档

适用版本：**WGT 1.1.0**（ABI 8） · Windows 10 / 11 x64 · Direct3D 11 / Direct3D 12

本文面向要把 WGT 接进自己游戏或工具的开发者：怎么配工程、每帧要调什么、输入和线程怎么处理、会碰到哪些坑。
完整接口列表见 [API.md](API.md)（按头文件列出所有类型、函数和选项）。设计原理见 [ARCHITECTURE.md](ARCHITECTURE.md)，扩展（自定义控件、主题、HLSL 效果、插件、自定义后端）见
[EXTENDING.md](EXTENDING.md)，线程规则的完整版见 [THREADING.md](THREADING.md)。

---

## 1. 一句话理解 WGT

WGT 是一个 DLL（`wgt.dll`）加一组头文件。**窗口、D3D 设备、交换链都是你游戏自己的**。WGT 只做三件事：

1. 从你的窗口过程接收输入（`HandleWin32Message`）；
2. 每帧由你调用控件函数（即时模式，和 Dear ImGui 一样的写法）；
3. 在你画完游戏画面之后，把 UI 合成到同一张后台缓冲上（`Render`），然后由你 `Present`。

```
启动    SetProcessDpiAwarenessContext(PMv2) → 创建窗口 / 设备 / 交换链 → wgt::Context::Create(desc)
窗口过程 每条消息先给 ui->HandleWin32Message(...)，返回 true 就说明 UI 用掉了
每帧    画游戏 → ui->NewFrame() → wgt::ui::* 控件 → ui->Render(后台缓冲) → Present
退出    停掉会访问 UI 的线程 → ui->Destroy() → 释放设备
```

最小完整示例：[`examples/minimal_d3d11/main.cpp`](../examples/minimal_d3d11/main.cpp)（单文件，约 250 行）。建议先把它跑起来，再照着接。

---

## 2. SDK 包内容

```
WGT-1.1.0-sdk-win64/
├─ bin/
│  ├─ wgt.dll                 运行时（随游戏发布）
│  ├─ wgt.pdb                 调试符号（不需要发布，崩溃分析时用）
│  ├─ wgt_demo.exe            功能展示 + 测试工具（见第 12 节）
│  ├─ wgt_minimal_d3d11.exe   最小接入示例
│  └─ plugins/wgt_plugin_hello.dll   插件示例（demo 启动时自动加载）
├─ include/
│  ├─ wgt/                    WGT 公共头文件，入口是 <wgt/wgt.hpp>
│  └─ imgui/                  WGT 版 Dear ImGui 头文件（imgui.h / imconfig.h / imgui_internal.h）
├─ lib/
│  ├─ wgt.lib                 导入库
│  └─ cmake/wgt/              CMake 包配置：find_package(wgt) → wgt::wgt
├─ wgt.props                  不用 CMake 的 Visual Studio 工程用的属性表
├─ docs/                      本文 + API（接口参考）/ ARCHITECTURE / EXTENDING / THREADING
├─ examples/                  minimal_d3d11、demo、plugin_hello 源码，可以直接对着 SDK 编译
├─ licenses/                  第三方许可证（Dear ImGui，MIT）
└─ README.md
```

发布游戏时只需要带上 **`wgt.dll`**（和你的插件 DLL，如果有的话）。

---

## 3. 环境要求

| 项目 | 要求 |
| --- | --- |
| 系统 | Windows 10 1703 及以上 / Windows 11，x64 |
| 显卡 | Direct3D 功能级别 11_0 及以上 |
| 编译器 | Visual Studio 2022（MSVC v143），C++20 |
| SDK | Windows 10/11 SDK（只用到系统自带的 D3D / DXGI 头文件） |
| 运行库 | Visual C++ 2015-2022 x64 运行库（`wgt.dll` 用的是 `/MD`），游戏安装包通常已经带了 |
| 可选 | `d3dcompiler_47.dll`（Windows 10+ 自带），只有用自定义 HLSL 效果时才需要 |

字体和图标都从系统取，不用打包资源：界面字体是 Segoe UI Variable（Win11）或 Segoe UI（Win10），
图标是 Segoe Fluent Icons（Win11）或 Segoe MDL2 Assets（Win10）；中日韩、阿拉伯文、emoji 等由
DirectWrite 自动回退到系统字体。

---

## 4. 工程配置

三种方式任选其一。无论哪种，最终都要做到这几点：包含目录有 `include` 和 `include/imgui`，定义
`IMGUI_USER_CONFIG="wgt/imconfig_wgt.h"`，使用 C++20 和 `/utf-8`，链接 `wgt.lib`，并把 `wgt.dll` 复制到 exe 旁边。

### 4.1 CMake（推荐）

```cmake
# 指向解压后的 SDK 根目录（也可以在命令行传 -DCMAKE_PREFIX_PATH=...）
list(APPEND CMAKE_PREFIX_PATH "D:/SDK/WGT-1.1.0-sdk-win64")
find_package(wgt 1.1 CONFIG REQUIRED)

target_link_libraries(MyGame PRIVATE wgt::wgt)   # 包含目录、宏定义、C++20、/utf-8 都随目标一起带上

# 构建后把 wgt.dll 复制到 exe 旁边
add_custom_command(TARGET MyGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:wgt::wgt> $<TARGET_FILE_DIR:MyGame>)
```

`find_package` 会检查版本（只接受 1.1.x）和位数（只接受 64 位工程）。SDK 里的 `examples/CMakeLists.txt`
就是这样对着 SDK 编译的，可以照抄：

```bash
cmake -S examples -B build-examples -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=D:/SDK/WGT-1.1.0-sdk-win64
```

### 4.2 Visual Studio 工程（属性表）

在“属性管理器”里给项目添加现有属性表 `wgt.props`（在 SDK 根目录），或者在 `.vcxproj` 里加一行：

```xml
<Import Project="D:\SDK\WGT-1.1.0-sdk-win64\wgt.props" />
```

它会设置包含目录、`IMGUI_USER_CONFIG`、C++20、`/utf-8`，链接 `wgt.lib`，并在生成后复制 `wgt.dll`。
所有配置（Debug / Release）都适用。

### 4.3 手动配置

| 位置 | 值 |
| --- | --- |
| C/C++ → 附加包含目录 | `<SDK>\include;<SDK>\include\imgui` |
| C/C++ → 预处理器定义 | `IMGUI_USER_CONFIG="wgt/imconfig_wgt.h"` |
| C/C++ → 语言 → C++ 语言标准 | `/std:c++20` |
| C/C++ → 命令行 → 其他选项 | `/utf-8` |
| 链接器 → 附加库目录 / 附加依赖项 | `<SDK>\lib` / `wgt.lib` |
| 生成事件 → 生成后事件 | `copy /Y "<SDK>\bin\wgt.dll" "$(OutDir)"` |

### 4.4 头文件与 Dear ImGui

* 统一用 `#include <wgt/wgt.hpp>`，它会包含全部公共头文件和 `imgui.h`。
* **WGT 自带一份改过的 Dear ImGui 1.92，编在 `wgt.dll` 里并导出**。你的工程里**不要**再编译 `imgui.cpp`
  等源文件，也不要链接另一份 ImGui，否则会出现重复符号或两个互不相通的 ImGui 上下文。也不要使用官方的
  `imgui_impl_win32` / `imgui_impl_dx11` / `imgui_impl_dx12` 后端，输入和渲染都由 WGT 负责。
* 已有的 ImGui 代码可以照常写（`ImGui::Begin`、`ImGui::DragFloat` ……），只要在 `NewFrame()` 和
  `Render()` 之间调用即可，外观会自动套用 WGT 主题，文字也走 WGT 的 DirectWrite 引擎。
* 如果某个 .cpp 先包含了 `imgui.h`、又没有定义 `IMGUI_USER_CONFIG`，编译会直接报错并提示原因。把宏加到
  工程级别（上面三种方式都会加）即可。
* 所有字符串都是 **UTF-8** 的 `const char*`，这也是要开 `/utf-8` 的原因（源码里的中文字面量才能按 UTF-8 编码）。

---

## 5. 接入 Direct3D 11

完整代码见 `examples/minimal_d3d11/main.cpp`，下面按步骤拆开讲。

### 5.1 DPI 感知（必须）

```cpp
SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);   // 在创建任何窗口之前
```

也可以写在应用清单里。**不设置的话，Windows 会把整个窗口按位图拉伸，文字一定发糊**。WGT 默认跟随窗口
所在显示器的 DPI，窗口拖到别的显示器时会自动重新缩放。

### 5.2 创建上下文

```cpp
#include <wgt/wgt.hpp>

wgt::ContextDesc desc;
desc.backend      = wgt::Backend::D3D11;
desc.hwnd         = hwnd;             // 用于输入、DPI、光标和输入法
desc.d3d11Device  = device;           // 你的设备
desc.d3d11Context = immediateContext; // Render() 用的立即上下文
desc.toggleKey    = VK_F1;            // 可选：一键显示 / 隐藏整个 UI
wgt::Context* ui = wgt::Context::Create(desc);   // 调用线程成为这个上下文的“UI 线程”

ui->SetLogCallback([](int level, const char* msg) {   // 0 信息 / 1 警告 / 2 错误，可能来自任意线程（调用是串行的）
    MyLog(level, msg);
});
```

`ContextDesc` 常用字段：

| 字段 | 默认 | 说明 |
| --- | --- | --- |
| `darkMode` / `accent` | `true` / 系统蓝 | 初始外观，运行时可用 `SetDarkMode` / `SetAccent` 改 |
| `uiScale` | 1.0 | 用户缩放偏好，和 DPI 相乘 |
| `dpiScale` | 0（跟随显示器） | 大于 0 时固定 DPI 缩放 |
| `renderScale` | 0（自动） | 渲染目标像素 / 窗口像素，见第 9.2 节 |
| `fontFamily` / `monoFamily` / `locale` | 系统默认 | 按字体族名取系统字体；`locale` 例如 `"zh-CN"`，影响回退字体和中日韩字形变体 |
| `fontFiles` / `fontFileCount` | 无 | 额外注册 .ttf / .otf / .ttc 文件 |
| `textAntialiasing` | `Auto` | 自动：系统开了 ClearType 且 1:1 显示时用次像素，否则灰度 |
| `iniFilename` | null | 窗口布局持久化文件，null 表示不保存 |
| `showDock` | true | 是否显示面板启动栏（Dock） |
| `toggleKey` | 0 | 显示 / 隐藏 UI 的虚拟键，0 表示不用 |
| `d3d11RestoreState` | true | Render 前后备份 / 恢复你的管线状态 |
| `maxBackdropCaptures` | 64 | 每帧液态玻璃背景捕获上限，超出会复用上一次捕获并记一次警告 |
| `debugLayout` | false | 布局检查器：红框标出互相重叠的控件，橙框标出被窗口右边缘截断的控件，并写日志 |

> **后端初始化失败时 `Create` 仍然返回上下文**，界面照常运行（布局、输入、文字测量），只是什么都不画。错误会写
> 日志（此时还没设日志回调，所以会出现在调试器的“输出”窗口里）。常见原因：设备指针为空，或者 D3D11 设备没有
> 立即上下文。`backend = wgt::Backend::None` 就是这种无渲染模式，可以用来跑自动化测试或在服务器上测量文字。

### 5.3 窗口过程

```cpp
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (g_ui && g_ui->HandleWin32Message(hwnd, msg, (std::uint64_t)wParam, (std::int64_t)lParam))
        return msg == WM_SETCURSOR ? TRUE : 0;       // UI 用掉了这条消息
    ...你原来的处理...
}
```

* **所有消息都转给它**，它自己会筛选。它是线程安全的，可以在窗口线程调用，而 UI 在另一个线程渲染。
* 返回值的含义（按上一帧的状态）：鼠标消息看 `WantsMouse()`，键盘消息看 `WantsKeyboard()`，`WM_CHAR`
  看是否正在输入文字；`WM_SETCURSOR` 在 UI 接管光标时返回 true；`toggleKey` 永远被吞掉；有 WGT 输入框
  获得焦点时，输入法消息（`WM_IME_*`）也会被吞掉，组字过程由 WGT 画在输入框里。
* UI 被隐藏（`SetVisible(false)` 或按了 `toggleKey`）时一律返回 false。
* 窗口在不同 DPI 的显示器之间移动时，处理一下 `WM_DPICHANGED`，采用 Windows 建议的窗口矩形就行，WGT 会自己重新缩放。

### 5.4 每帧

```cpp
// 1) 先画游戏（场景、后处理、色调映射……），最终写进后台缓冲
RenderGame(backBufferRtv);

// 2) 游戏只处理 UI 没有拿走的输入
if (!ui->WantsMouse())    game.HandleMouse();
if (!ui->WantsKeyboard()) game.HandleKeyboard();

// 3) UI：即时模式，每帧都调用
ui->NewFrame();
static bool open = true, vsync = true;
static float volume = 80.0f;
if (wgt::ui::BeginWindow("Settings", &open))
{
    if (wgt::ui::BeginSection("Display"))
    {
        wgt::ui::RowToggle("VSync", &vsync, {wgt::icons::Refresh, wgt::Color::Hex(0x30D158)});
        wgt::ui::RowSlider("Volume", &volume, 0, 100, {wgt::icons::Volume});
        wgt::ui::EndSection();
    }
    wgt::ui::ButtonOptions b;
    b.kind = wgt::ui::ButtonKind::Glass;
    if (wgt::ui::Button("Apply", b))
        ApplySettings();
    wgt::ui::EndWindow();
}
ui->Render(wgt::RenderTarget::D3D11(backBufferRtv));   // 合成到游戏画面上

// 4) 交换
swapChain->Present(vsync ? 1 : 0, 0);
```

要点：

* `NewFrame()` 和 `Render()` 每帧**各调用一次，成对出现**，并且在同一个线程（创建上下文的那个线程）。
* `Render()` 一定要放在游戏画面画完**之后**：液态玻璃模糊的就是当前后台缓冲里已有的内容。
* 默认情况下（`d3d11RestoreState = true`）WGT 会备份并恢复你的管线状态，`Render` 之后可以直接继续画。
  关掉可以省一点 CPU，但之后需要自己重新绑定状态。
* `BeginXxx()` 返回 true 时才调用对应的 `EndXxx()`（`BeginWindow` / `BeginSection` / `BeginFlow` 等都一样）。

### 5.5 窗口大小变化

WGT **不持有交换链缓冲的引用**，内部的屏幕大小纹理也会按需自动重建。所以照常处理就行：

```cpp
backBufferRtv.Reset();
context->ClearState();
swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, flags);
CreateBackBufferRtv();
```

`ui->InvalidateDeviceObjects()` 只是立即释放 WGT 与屏幕尺寸相关的 GPU 资源（下次用到时重建），比如想回收显存时调用；
改变窗口大小时**不需要**调用它。

### 5.6 退出

```cpp
StopThreadsThatUseTheUi();   // 先停掉还在调用 ui->Notify / Post / Property 的线程
g_ui = nullptr;              // 窗口过程不再转发
ui->Destroy();               // 在释放 D3D 设备之前
```

### 5.7 设备丢失 / 重建设备

上下文和设备绑定。设备被移除（`DXGI_ERROR_DEVICE_REMOVED`）或者要换设备时：`Destroy()` 旧上下文 →
新建设备 → `Create()` 新上下文，再重新注册用 `CreateTexture` 建的贴图、`AddPanel` 加的面板和插件。

---

## 6. 接入 Direct3D 12

参考实现：`examples/demo/host_d3d12.cpp`。

```cpp
wgt::ContextDesc desc;
desc.backend = wgt::Backend::D3D12;
desc.hwnd = hwnd;
desc.d3d12Device = device;
desc.d3d12FramesInFlight = 3;   // >= 你同时在飞的帧数（通常等于后台缓冲数）
wgt::Context* ui = wgt::Context::Create(desc);
```

每帧：

```cpp
WaitForFrameFence(frameIndex);     // 先等这一帧的资源空出来，再 NewFrame（输入更新鲜，统计也不含等待）
allocator->Reset();
cmdList->Reset(allocator, nullptr);
Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
RecordGame(cmdList);               // 画游戏

ui->NewFrame();
BuildUi();
ui->Render(wgt::RenderTarget::D3D12(cmdList, backBuffer, rtvHandle.ptr, DXGI_FORMAT_R8G8B8A8_UNORM));

// 如果之后还要在同一个列表里录制：重新设置你的描述符堆、根签名、PSO、视口 / 裁剪、渲染目标
Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
cmdList->Close();
queue->ExecuteCommandLists(1, lists);
swapChain->Present(syncInterval, flags);
queue->Signal(fence, ++fenceValue);
```

规则：

* **命令列表必须是 DIRECT 类型、处于录制状态**；后台缓冲必须处于 `D3D12_RESOURCE_STATE_RENDER_TARGET`，
  WGT 结束时保持这个状态不变。
* WGT 只往你的列表里录制，**不创建提交用的队列、不提交、不等待**。贴图和字形的上传也录在同一个列表里，
  所以这个列表必须被执行。
* WGT 会设置自己的描述符堆、根签名、PSO、图元拓扑、视口 / 裁剪和渲染目标，`Render` 之后需要你自己恢复。
* WGT 的每帧上传内存和描述符按 `d3d12FramesInFlight` 轮换回收。这个值**小于**你实际在飞的帧数时，会出现
  GPU 读到被覆盖的数据（闪烁、花屏甚至设备移除）。
* `rtvFormat` 传 RTV 的格式；传 0 表示用资源本身的格式（资源是 TYPELESS 时必须显式传）。

---

## 7. 输入

### 7.1 和游戏分配输入

```cpp
if (!ui->WantsMouse())    game.HandleMouse();      // 鼠标在 UI 上、正在拖动滑块……时为 true
if (!ui->WantsKeyboard()) game.HandleKeyboard();   // 输入框有焦点等情况为 true
```

这两个函数任意线程都能调用，反映的是上一帧的状态。UI 隐藏时都是 false。

### 7.2 Raw Input / DirectInput

WGT 只读取标准窗口消息（`WM_MOUSEMOVE`、`WM_LBUTTONDOWN`、`WM_KEYDOWN`、`WM_CHAR`……）。如果游戏用
`RIDEV_NOLEGACY` 注册了 Raw Input，系统就不会再生成这些消息，UI 会收不到输入。解决办法任选其一：
UI 可见时去掉 `NOLEGACY`；或者用注入接口把输入转给 UI：

```cpp
ui->InjectMousePos(x, y);            // 窗口客户区像素
ui->InjectMouseButton(0, true);      // 0 左 / 1 右 / 2 中
ui->InjectMouseWheel(1.0f);
ui->InjectKey(ImGuiKey_Enter, true);
ui->InjectText("你好");            // UTF-8（开了 /utf-8 的普通字面量即可，C++20 的 u8"" 是 char8_t）
ui->EndInputInjection();             // 结束注入，恢复系统鼠标位置
```

注入接口任意线程可调用，在下一个 `NewFrame` 生效，也可以用来做自动化 UI 测试。

### 7.3 显示 / 隐藏、光标、输入法

* `SetVisible(bool)` / `ToggleVisible()` / `IsVisible()`：隐藏时不画界面，不接收输入，但通知岛（`Notify`）照常显示。
* 光标形状由 WGT 在 `WM_SETCURSOR` 里设置（所以那里要返回 TRUE）。游戏自己隐藏光标的话，UI 可见时记得恢复。
* 中文 / 日文 / 韩文输入法开箱即用：组字内容直接画在 WGT 输入框里，候选框跟随光标。前提是
  `HandleWin32Message` 能收到 `WM_IME_*` 消息，而且不要对窗口调用 `ImmAssociateContext(hwnd, nullptr)`。

### 7.4 窗口线程与渲染线程分离

很多引擎在主线程处理消息、在渲染线程画图，WGT 直接支持这种结构：`HandleWin32Message` 在窗口线程调用，
`Create` / `NewFrame` / `Render` / `Destroy` 在渲染线程调用。键盘状态、鼠标捕获、离开跟踪和输入法都在正确
的线程上处理，修饰键和拖出窗口的操作都正常。`examples/demo/main.cpp` 就是这样跑的。

---

## 8. 线程

规则只有一条：**一个线程负责构建和渲染 UI（UI 线程），其他线程通过线程安全的接口和它通信。**

| 只能在 UI 线程 | 任意线程 |
| --- | --- |
| `NewFrame`、`Render`、`Destroy`、`InvalidateDeviceObjects` | `HandleWin32Message`、`WantsMouse`、`WantsKeyboard`、`Inject*` |
| 所有 `wgt::ui::*` 控件、`wgt::Painter`、`wgt::anim::*` | `Notify`、`SetActivity`、`ClearActivity` |
| `ImGui::*` | `SetTheme`、`SetDarkMode`、`SetAccent`、`SetUiScale`、`SetDpiScale`、`SetRenderScale` |
| `GetTheme`、`CurrentTheme`、`GetFont`、`MeasureText`、`RegisterFont` | `AddPanel`、`RemovePanel`、`SetPanelOpen`、`IsPanelOpen` |
| | `RegisterEffect`、`AddFontFile`、`CreateTexture`、`DestroyTexture` |
| | `LoadPlugin`、`LoadPluginsFromDirectory`、`AddPlugin`、`Post`、`GetStats`、`SetVisible` |

面板回调、`Post` 的任务和插件回调都在 UI 线程执行，即使它们是从别的线程注册的。

游戏线程往 UI 传数据用 `wgt/sync.hpp` 里的三个纯头文件类型（跨 DLL 安全）：

```cpp
wgt::Property<float> health{1.0f};        // 可观察值：小类型无锁
health.Set(0.75f);                        // 游戏线程
wgt::ui::ProgressBar(health.Get());       // UI 线程

wgt::Channel<std::string> chat;           // 多生产者 / 单消费者队列
chat.Push("玩家 A 加入了游戏");            // 任意线程
chat.Drain([](std::string& line) { lines.push_back(std::move(line)); });   // UI 线程，每帧

wgt::Latest<NetStats> net;                // 无锁三缓冲：UI 永远读到最新快照
net.Publish(sample);                      // 生产者线程
NetStats s; if (net.Fetch(s)) { /* 有新数据 */ }

ui->Post([=] { /* 下一帧开始时在 UI 线程执行 */ });
```

其他注意：`GetTheme()` 返回的引用只在当前帧有效，要长期保存就复制一份 `Theme`；`Destroy()` 之前先停掉所有生产者线程。
多个上下文可以在多个线程上同时运行（比如游戏 HUD 和工具窗口），详见 [THREADING.md](THREADING.md)。

---

## 9. 渲染相关

### 9.1 支持的渲染目标格式

| 格式 | 说明 |
| --- | --- |
| `R8G8B8A8_UNORM` / `B8G8R8A8_UNORM` / `B8G8R8X8_UNORM` | 常规 |
| 上面三种的 `_SRGB` 视图 | WGT 输出线性值，由硬件做 sRGB 编码 |
| `R10G10B10A2_UNORM` | 支持 |
| `R16G16B16A16_FLOAT` | 支持，但按与 UNORM 相同的 gamma 编码值写入。scRGB / HDR 管线请把 UI 画在最终的 SDR 缓冲上，或者单独画到一张 8 位中间纹理再合成 |
| MSAA 目标 | 支持（捕获背景时自动 resolve） |

UI 应该画在色调映射和后处理**之后**，也就是最终要 `Present` 的那张缓冲上。

### 9.2 坐标与缩放

* **UI 单位 = 窗口客户区像素**，鼠标坐标、`WindowOptions::pos` / `size` 等都用这个单位。
* **度量缩放 = DPI × uiScale**：所有尺寸、圆角、字号都会乘上它。自定义控件里用 `wgt::ui::S(12)` 取缩放后的值。
* **渲染缩放 = 渲染目标像素 / UI 单位**：默认自动检测。游戏以超采样（渲染目标比窗口大）或降采样输出时，
  文字按实际像素密度光栅化，依然清晰。可用 `SetRenderScale` 固定。

### 9.3 性能参考

RTX 4080 SUPER，2400×1231，Release 版，D3D11，vsync 关，默认 Frosted 观感（`wgt_demo --stats` 实测，三次取范围）：

| 场景 | 玻璃捕获 | UI CPU | UI GPU |
| --- | --- | --- | --- |
| 只有 Dock | 2 | 0.06 ms | 0.08–0.09 ms |
| 设置面板 | 2 | 0.16–0.17 ms | 0.15–0.16 ms |
| 三个面板（含控制中心） | 15 | 0.19–0.20 ms | 0.49–0.54 ms |
| 全部面板 | 20 | 0.31 ms | 0.67–0.75 ms |

约九成 GPU 时间花在液态玻璃上：一半是背景捕获（复制 + 模糊金字塔），一半是玻璃本身的着色。同一个窗口里的玻璃控件会共用一次捕获。
`ui->GetStats()` 可以随时读到这些数字（见第 12 节）。

**性能建议：让渲染目标可以被采样。** 渲染目标带着色器资源绑定时（D3D11：`D3D11_BIND_SHADER_RESOURCE`；
交换链：`BufferUsage |= DXGI_USAGE_SHADER_INPUT`；D3D12：资源没有 `D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE`），
雾面玻璃的背景捕获会直接从渲染目标构建模糊金字塔，省掉整块复制（实测捕获耗时少 15–20%），画面完全一样。
不带时自动退回复制，功能不受影响。引擎自己的离屏渲染目标通常本来就有这个绑定；MSAA 目标和类型为 `_SRGB` 的纹理走复制。

### 9.4 帧率与垂直同步

VSync 由你的 `Present` 决定。游戏没有自己的限帧器时，可以用 `wgt::FramePacer`（高精度可等待计时器 +
最后约 1 ms 自旋，帧时间精确，不空转占满 CPU）：

```cpp
wgt::FramePacer pacer;
pacer.SetTargetFps(120);          // 0 = 不限；只在目标改变时调用
while (running) { ...; swapChain->Present(0, 0); pacer.Wait(); }
```

---

## 10. 常用功能速查

```cpp
// 通知（任意线程）：像 Siri 光球一样弹出，先展开成完整卡片（液态玻璃透镜 + 分合变色的光束），
// 约一秒后变形成普通灵动岛胶囊，驻留到通知结束
wgt::Notification n;
n.title = "下载完成"; n.message = "材质包已安装"; n.icon = wgt::icons::Download;
ui->Notify(n);

// 折线图：一条连贯的线（平滑、不越过数据点）+ 下方渐变；glow 默认关闭，要发光就打开
wgt::ui::LineChartOptions co;
co.offset = cursor;          // 环形缓冲区里最旧的一个
co.glow = false;             // true: 整条线一个均匀的光晕
wgt::ui::LineChart("fps", history, 120, co);
// 进度条 / 进度环：ProgressOptions，默认不发光（glow = true 才发光）
wgt::ui::ProgressOptions po;
po.tint = wgt::Color::Hex(0x30D158);
po.diameter = 54;
po.glow = false;
wgt::ui::ProgressRing(load, po);
wgt::ui::ProgressBar(health.Get(), {.glow = true});   // C++20 指定初始化也可以
// 自己画：Painter::Polyline（一笔画完，发光与否由 Style 决定）/ Painter::Area（下方填充）

// 实时活动：灵动岛里的进度（progress < 0 表示不确定进度）
ui->SetActivity("dl", "正在下载", 0.42f, wgt::icons::Download);
ui->ClearActivity("dl");

// 悬浮玻璃搜索栏：固定在当前页面底部，列表从它下面滚过（iOS 设置）。透明玻璃 + 柔和底色（默认开）；
// base = false 去掉底色，fill 自定义底色颜色，look 可改成 Frosted（iOS 雾面）或 Theme（主题的栏材质）
wgt::ui::SearchBarOptions sbo;
sbo.base = true;                      // 默认就是开；false = 只剩玻璃
wgt::ui::SearchBar("search", searchBuf, sizeof(searchBuf), sbo);

// 单个组件单独改样式：在组件前一行 ui::Next()，只改你设的项，其余跟主题走，不用改主题、不用 Push/Pop
wgt::ui::Next().Tint(wgt::Color::Hex(0x30D158)).Radius(8);            // 强调色 + 圆角
wgt::ui::Button("开始游戏");
wgt::ui::Next().Look(wgt::GlassLook::Frosted).Blur(24).Fill(purple);   // 实心按钮变成紫色雾面玻璃
wgt::ui::Button("雾面");
wgt::ui::Next().Look(wgt::GlassLook::Clear).Refraction(20);           // 全透明 + 强透镜
wgt::ui::Slider("vol", &vol, 0, 1);
wgt::ui::Next().Opacity(0.5f);                                        // 整个组件半透明
wgt::ui::Toggle("wifi", &wifi);
// 可设：Look（Theme / Clear 全透明 / Frosted 雾面）、玻璃参数 Blur / Refraction / Bezel / Dispersion /
// Saturation / Brightness / Specular / Legibility / Magnify / GlassTint / Rim（或 Glass(整个材质)）、
// Tint（强调色）、Fill（底色）、Radius（圆角）、Opacity（不透明度）、Label（文字颜色）；
// 分段控件 / 标签栏：SelectedFill（切换后选中项的背景）、MovingFill（切换过程中移动的透镜颜色）、SelectedLabel（选中项文字）
wgt::ui::Next().Fill(blue).SelectedFill(wgt::Color::White(0.92f)).MovingFill(wgt::Color::White(0.25f))
               .Label(wgt::Color::White()).SelectedLabel(blue);
wgt::ui::Segmented("range", &range, items, 3);
// 放在窗口 / 分组 / 卡片 / BeginRow 前：它和里面的所有组件
wgt::ui::Next().Look(wgt::GlassLook::Clear);
if (wgt::ui::BeginSection("音频")) { /* ... */ wgt::ui::EndSection(); }
// 一段范围：
{ wgt::ui::StyleScope s(wgt::ui::ItemStyle().Tint(red).Radius(6)); /* ... */ }
ui->SetGlassLook(wgt::GlassLook::Theme);            // 整个界面的默认观感（任意线程）；默认 Frosted（iOS 雾面）
// Clear 下：按钮底色、开关 / 滑块轨道、分段控件、输入框、徽章、分组 / 卡片背景、窗口都变成透明玻璃，
// 开关的“开”、选中项、进度这些状态色保留但变成半透明
// 面板的窗口背景：PanelDesc::flags |= PanelFlags_ClearGlass（ui::BeginWindow 用 WindowFlags_ClearGlass）
// 自定义控件：ui::Interact 返回的 Interaction::style 就是它该用的样式，绘制时 ui::StyleScope s(it.style);

// 面板：出现在 Dock 里，由 WGT 负责窗口、开关和布局
wgt::PanelDesc pd;
pd.id = "inventory"; pd.title = "背包"; pd.icon = wgt::icons::Shop;
pd.size = wgt::Vec2(420, 560);
ui->AddPanel(pd, [](wgt::Context& ctx) { wgt::ui::Text(wgt::TextStyle::Body, "..."); });
ui->SetPanelOpen("inventory", true);

// 外观与缩放（任意线程，带动画）
ui->SetDarkMode(false);
ui->SetAccent(wgt::Color::Hex(0xFF9F0A));
ui->SetUiScale(1.25f);

// 贴图（RGBA8），可用于 ui::Image、Painter::Image、Style::Image
ImTextureID tex = ui->CreateTexture(pixels, w, h);
wgt::ui::Image(tex, wgt::Vec2(128, 128), 12.0f);
ui->DestroyTexture(tex);

// 字体（详见 10.1）
ui->AddFontFile(L"assets/fonts/MyBrand.ttf");        // 下一帧生效
wgt::FontId brand = wgt::RegisterFont("My Brand", 700);   // 可以紧接着调用：文件生效前先用界面字体，之后自动切换
p.Text(pos, wgt::FontRef{brand, wgt::ui::S(28)}, color, "GAME OVER");

// 自动布局：按可用宽度换行 / 分列，窗口大小变化时带弹簧动画重排
wgt::ui::GridOptions g; g.minColumnWidth = 160;
if (wgt::ui::BeginGrid("items", g)) { for (auto& it : items) DrawItem(it); wgt::ui::EndGrid(); }

// 自定义控件：布局 + 交互 + 绘制
wgt::ui::Interaction it = wgt::ui::Interact("card", wgt::Vec2(wgt::ui::AvailableWidth(), wgt::ui::S(120)));
if (it.visible)
{
    wgt::Painter p;
    p.Rect(it.rect, wgt::Style().Radius(wgt::ui::S(20))
                        .Glass(ui->GetTheme().materials.control)
                        .Shadow(wgt::Color::Black(0.25f), wgt::ui::S(18), wgt::Vec2(0, wgt::ui::S(6))));
    if (it.pressed) OpenCard();
}

// 自定义 HLSL 效果（注册后在后台编译，约 1 秒；编译好之前用内置着色器绘制）
wgt::EffectId aurora = ui->RegisterEffect("aurora", kAuroraHlsl);
p.Rect(r, wgt::Style().Radius(18).Effect(aurora, 1.0f));
```

控件、选项和逐组件样式的完整列表见 [API.md](API.md)（头文件 `include/wgt/ui.hpp` 里每个函数也都有注释）。更多写法（主题 token、材质、动画、布局细节）见 [EXTENDING.md](EXTENDING.md)。

### 10.1 自定义字体

字体按**字体族名**引用。系统里装了的直接写名字；游戏自带的 .ttf / .otf / .ttc 先注册文件，注册后它和系统字体
在同一个字体集合里，用法完全一样。

**整个界面换成自己的字体**（在创建时指定）：

```cpp
const wchar_t* files[] = {L"assets/fonts/MyBrand-Regular.ttf", L"assets/fonts/MyBrand-Bold.ttf"};
desc.fontFiles = files;
desc.fontFileCount = 2;
desc.fontFamily = "My Brand";              // 所有 WGT 控件和 ImGui 原生控件都用它
desc.fontFamilyDisplay = "My Brand Display";   // 可选：大字号（标题）用的光学尺寸字体族
desc.monoFamily = "JetBrains Mono";        // 可选：等宽
desc.iconFamily = "My Icons";              // 可选：图标字体，wgt::Icon 就是码点
```

**局部使用额外的字体**（标题、数字、品牌字样……）：

```cpp
ui->AddFontFile(L"assets/fonts/Digits.ttf");               // 任意线程，下一帧生效
wgt::FontId digits = wgt::RegisterFont("Digits", 700);     // UI 线程；同一个 id 一直有效
wgt::Painter p;
p.Text(pos, wgt::FontRef{digits, wgt::ui::S(40)}, color, "12:48");
wgt::Vec2 size = wgt::MeasureText(wgt::FontRef{digits, wgt::ui::S(40)}, "12:48").size;
```

规则：

* 字体里没有的字符（比如英文字体里的中文、emoji）**逐字回退到系统字体**，不会显示成方块。
* 字重用 100..950（400 常规、600 半粗、700 粗）。字体本身没有这个字重时，DirectWrite 会选最接近的，或合成加粗。
* 字体文件只是被引用，不会拷贝进内存：**上下文存活期间文件要一直在磁盘上**。
* 找不到的字体族会回退到界面字体（不报错），名字请以字体文件里的“字体族名称”为准（Windows 字体预览窗口里能看到）。
* `wgt::ui::*` 控件按主题的文字样式（`TextStyle`）取字体，也就是 `desc.fontFamily`；单个控件用别的字体，请用
  `Painter::Text` / `TextBox` 自己画，或者写自定义控件。
* 目前的限制：界面字体（`fontFamily`）只能在创建时指定，运行时切换需要重建上下文；还不支持直接从内存（资源包）
  加载字体，需要先解到一个文件。
* 字体授权由你负责。只用系统已安装的字体来显示是没问题的，但不要把系统字体文件打包进游戏分发。

---

## 11. 插件

插件是独立的 DLL，可以往任意 WGT 宿主里加面板、效果和每帧逻辑：

```cpp
// my_plugin.cpp —— 链接 wgt.lib，只用公共头文件
#include <wgt/wgt.hpp>
class MyPlugin final : public wgt::IPlugin
{
public:
    const char* Name() const override { return "My Plugin"; }
    void OnAttach(wgt::Context& ctx) override { /* AddPanel、RegisterEffect ... */ }
    void OnFrame(wgt::Context& ctx) override { /* 每帧，在 UI 线程 */ }
    void OnDetach(wgt::Context& ctx) override { ctx.RemovePanel("my.panel"); }
};
WGT_DECLARE_PLUGIN(MyPlugin)
```

宿主调用 `ui->LoadPluginsFromDirectory(L"plugins")` 或 `ui->LoadPlugin(L"plugins/my_plugin.dll")`。
加载时会检查 `WGT_ABI_VERSION`，版本不一致的插件会被拒绝并写日志。完整示例：`examples/plugin_hello`。

---

## 12. 诊断与排错工具

* **日志**：`SetLogCallback`，级别 0 信息 / 1 警告 / 2 错误。着色器编译错误、玻璃捕获超预算、插件加载失败都会写在这里。
* **统计**：`ui->GetStats()` 返回 `FrameStats`：`cpuUiMs`（NewFrame 到 Render 的构建耗时）、`gpuMs` /
  `gpuGlassMs` / `gpuLayerMs`（GPU 时间戳，D3D11 / D3D12 都支持，数据有几帧延迟，不会造成阻塞）、
  `drawCalls`、`fxInstances`、`backdropCaptures`、`glowLayers`、`vertices`。
* **布局检查器**：`desc.debugLayout = true` 或 `ui->SetDebugLayout(true)`。互相重叠的控件用红框标出；超出窗口右边缘、被截掉的控件用橙框标出（持续约半秒才算，页面切换滑动之类的过程不会误报）。每个问题只记一次日志。
* **避免溢出**：`ui::Text` 系列文字比这一行剩下的宽度还宽时，会在容器边缘自动换行（包括跟在 `SameLine` 后面的文字）。需要并排、又不能被截掉的一组控件，请放进 `ui::BeginFlow` / `BeginAdaptiveStack`，放不下时会自动换到下一行；直接用 `ImGui::SameLine` 的话，控件放不下也照样摆在那里，会被窗口截掉。
* **D3D 调试层**：WGT 在两个 API 的调试层下都是 0 警告 / 0 错误。你打开调试层后看到的警告如果来自 WGT，请反馈。
* **GPU 耗时拆分**：设置环境变量 `WGT_GPU_PROFILE=1`（D3D11）后，每 240 帧往日志（级别 2）写一行平均值：背景捕获、
  玻璃着色、普通形状、文字 / ImGui 几何、发光层各占多少毫秒，另外列出某一帧每次捕获的区域大小。调优界面时用。
* **版本检查**：头文件和 DLL 不匹配会导致难以理解的崩溃，建议启动时检查一下：
  ```cpp
  if (wgt::GetAbiVersion() != WGT_ABI_VERSION) { /* wgt.dll 与头文件不是同一版本 */ }
  ```
* **wgt_demo.exe**：既是展示也是测试工具。

  | 参数 | 作用 |
  | --- | --- |
  | `--dx11` / `--dx12` | 选择后端 |
  | `--open-all` / `--open settings,effects,control,components,languages,telemetry` | 打开面板 |
  | `--novsync`、`--fps 120` | 关闭垂直同步、限帧 |
  | `--stats --frames 600` | 跑 600 帧后把平均 CPU / GPU 数据追加到 `wgt_stats.txt` |
  | `--debug-layer` | 打开 D3D 调试层，结果写入 `wgt_debug_layer.txt` |
  | `--screenshot out.png --frames 120` | 第 N 帧截图后退出 |
  | `--scale 1.5`、`--render-scale 2`、`--light`、`--text gray` | 缩放、渲染缩放、浅色模式、灰度文字 |

---

## 13. 二进制兼容性

* 公共 API 导出了 C++ 类，所以**只支持 MSVC x64**。已验证的是 Visual Studio 2022（v143 工具集），更早的工具集未测试。
* **Debug 版游戏可以直接用 Release 版 `wgt.dll`**：跨 DLL 的回调用的是 `wgt::Callback`（三个裸指针，
  闭包在调用方模块里分配和释放），接口上没有 STL 类型，`Property` / `Channel` / `Latest` 都是纯头文件。
  SDK 里的示例在 Debug 和 Release 下都按这种方式测试过。
* `WGT_ABI_VERSION` 在导出结构体或接口的布局变化时递增。**次版本号变化（1.1 → 1.2）时请重新编译**游戏和插件；
  CMake 包的版本检查只接受同一个次版本。

### 13.1 本版默认行为与接口变化（ABI 8）

用之前的 1.1.0 包接入过的项目，请换上新包后**重新编译**，并留意下面几点：

| 变化 | 旧行为 | 新行为 | 想保留旧效果 |
| --- | --- | --- | --- |
| 玻璃观感默认值 | `Theme`（主题材质） | `Frosted`（iOS 分层雾面） | `ui->SetGlassLook(wgt::GlassLook::Theme)` |
| 进度条 / 进度环 | 固定带光晕 | 默认不发光 | 用 `ProgressOptions`，设 `glow = true` |
| `ui::Text` 系列 | 不换行，超长会伸出窗口 | 比剩余宽度还宽时在容器边缘自动换行（自动布局容器里仍由容器决定） | 用 `PushTextWrapPos` 控制换行位置 |
| `RowSlider` 的数值 | 超出范围时显示原值，数值越长滑块越短，可能挤出这一行 | 显示夹到范围内的值（与滑块位置一致）；数值列按 min / max 定宽；窄时数值移到标签行；滑块始终留在行内 | — |
| `BeginRow` / `BeginCard` 里填满宽度的控件 | 按窗口内容宽度计算，可能超出行 / 卡片 | 限制在行 / 卡片的内容区内 | — |
| 搜索栏 | 自带底部白雾 | 透明玻璃 + 柔和底色（`SearchBarOptions::base`，默认开）；`fill` 自定义底色，`look` 可选观感 | `sbo.base = false` 只剩玻璃 |
| 单个组件的样式 | 只能改主题（整类组件一起变） | 任意组件单独改：`ui::Next()` 一行（玻璃观感 / 玻璃参数 / 强调色 / 底色 / 圆角 / 不透明度 / 文字颜色，分段控件和标签栏还有选中背景、切换中的透镜颜色、选中文字），`ui::PushItemStyle` / `StyleScope` 一段范围 | — |
| 透明选项 | 只有窗口 / 面板 / 搜索栏等少数组件能设 | 任意组件：`ui::Next().Look(GlassLook::Clear)`（下一个组件）、作用域、`SetGlassLook`（全局） | — |
| `ui::Interaction` / `ui::ItemStyle` | — | `Interaction` 新增 `style` 字段，`ItemStyle` 是跨 DLL 传递的结构体（布局变化，所以 ABI 升到 8） | 重新编译 |
| `Painter::LightStreak` | `(…, smile, wave)` | `(…, smile, open, time)`：open = 中间张开的幅度，time = 动画自己的时钟（< 0 用帧时钟） | 按新参数含义调用 |
| 新增 | — | `ui::LineChart` / `LineChartOptions`、`ProgressOptions`、`Painter::Polyline` / `Painter::Area`、`PolylineFlags_Smooth`、`ui::PushGlassLook` / `PopGlassLook` / `LookMaterial`、`WindowFlags_ClearGlass` / `PanelFlags_ClearGlass`、`Painter::BeginEdgeFade` / `EndEdgeFade`、`Paint::Spectrum` / `Paint::ConicLoop` | — |

---

## 14. 常见问题

**文字发糊。**
99% 是 DPI 感知没开（见 5.1）。其次检查是不是把 UI 画到了比窗口小的渲染目标上（降采样渲染），这时文字按更低的
像素密度光栅化，应改为画到最终分辨率的后台缓冲上。

**编译报错：`imgui.h was included before wgt headers ...`。**
这个 .cpp 先包含了 `imgui.h`。在工程级别定义 `IMGUI_USER_CONFIG="wgt/imconfig_wgt.h"`，或者先包含 `<wgt/wgt.hpp>`。

**链接报 ImGui 符号重复定义。**
工程里还编译着一份 `imgui*.cpp` 或链接了别的 ImGui 库，删掉即可，ImGui 由 `wgt.dll` 提供。

**UI 不显示。**
依次检查：每帧是否都调用了 `Render`；是否被 `toggleKey` 或 `SetVisible(false)` 隐藏了；调试器输出窗口里有没有
“render backend failed to initialize”；D3D11 传入的 RTV 是否是当前后台缓冲的；D3D12 的命令列表是否被执行了。

**点 UI 的时候游戏也响应了。**
游戏侧的输入要用 `WantsMouse()` / `WantsKeyboard()` 过滤（见 7.1）。窗口过程里 `HandleWin32Message` 返回 true 的消息不要再交给游戏。

**UI 收不到鼠标 / 键盘。**
确认每条消息都转给了 `HandleWin32Message`、`hwnd` 是同一个窗口；用了 Raw Input 的看 7.2。

**液态玻璃是黑的，或者没有模糊出游戏画面。**
`Render` 调早了（在游戏画面画完之前），或者渲染目标格式不在 9.1 的列表里。玻璃非常多的场景如果日志里出现
“capture budget exceeded”，可以调大 `maxBackdropCaptures`。

**D3D12：Render 之后我自己的绘制乱了。**
WGT 改了描述符堆、根签名等状态，之后要重新设置（见第 6 节）。

**D3D12：偶尔闪烁、花屏或设备移除。**
`d3d12FramesInFlight` 小于实际在飞的帧数。

**中文 / emoji 显示成方块。**
正常情况下不会，DirectWrite 会自动回退到系统字体。如果指定了自定义 `fontFamily`，确认该字体已安装或已用
`AddFontFile` 注册；`locale` 会影响回退字体的选择（比如 `"zh-CN"` 和 `"ja-JP"` 选出的汉字字形不同）。

**自定义 HLSL 效果刚开始不显示。**
效果在后台编译，大约需要 1 秒，期间用内置着色器绘制。一直不显示的话，看日志里的编译错误。

**多个窗口都要 UI。**
每个窗口一个 `Context`，各自传自己的 `hwnd` 和渲染目标。它们可以在不同线程上同时运行；同一个线程驱动多个
上下文时，用 `wgt::SetCurrentContext()` 切换。

---

## 15. 接入检查清单

- [ ] 进程设置了 Per-Monitor V2 DPI 感知
- [ ] 工程：包含目录 `include` 和 `include/imgui`、`IMGUI_USER_CONFIG`、C++20、`/utf-8`、`wgt.lib`
- [ ] `wgt.dll` 复制到 exe 旁边，安装包里也带上了
- [ ] 没有再编译或链接另一份 Dear ImGui 或官方 ImGui 后端
- [ ] 窗口过程把所有消息先交给 `HandleWin32Message`，`WM_SETCURSOR` 返回 TRUE
- [ ] 游戏输入用 `WantsMouse()` / `WantsKeyboard()` 过滤
- [ ] 每帧：画游戏 → `NewFrame` → 控件 → `Render` → `Present`，都在同一个 UI 线程
- [ ] D3D12：`d3d12FramesInFlight` ≥ 在飞帧数；`Render` 之后重新设置自己的描述符堆和根签名
- [ ] 改变窗口大小时释放 RTV、`ClearState()` 后再 `ResizeBuffers`（不需要调用 WGT 的任何函数）
- [ ] 退出：先停掉生产者线程，再 `Destroy()`，最后释放设备
- [ ] 设置了日志回调，开发期打开过一次 D3D 调试层，确认没有警告

---

## 16. 从源码构建与打包

```bash
cmake --preset vs2022
```

```bash
cmake --build --preset release
```

```bash
cmake --install build --config Release --prefix D:/SDK/WGT-1.1.0-sdk-win64
```

```bash
cpack --config build/CPackConfig.cmake -C Release
```

```bash
cpack --config build/CPackSourceConfig.cmake
```

最后两条分别在 `build/packages/` 里生成 SDK 包 `WGT-1.1.0-sdk-win64.zip` 和源码包 `WGT-1.1.0-src.zip`。
`-DWGT_BUILD_EXAMPLES=OFF` 只构建库本身。
