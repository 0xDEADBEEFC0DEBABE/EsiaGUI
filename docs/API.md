# WGT API 参考

适用版本：**WGT 1.1.0**（ABI 8）。本文按头文件列出 SDK 的全部公开接口；接入步骤见 [INTEGRATION.md](INTEGRATION.md)，扩展写法见 [EXTENDING.md](EXTENDING.md)，线程模型见 [THREADING.md](THREADING.md)。

**约定**

* **线程**：`[任意线程]` 表示可以在游戏线程或工作线程直接调用（排队或加锁，下一帧生效）；没有标注的都只能在 **UI 线程**（调用 `NewFrame()` / `Render()` 的线程）上、两者之间调用。
* **单位**：尺寸、圆角、字号都是 **UI 单位**（缩放为 1 时等于窗口客户区像素）。`Options` 结构体里的尺寸按“缩放为 1 时的设计尺寸”填写，WGT 内部会乘上 DPI 和用户缩放；`Painter` 的坐标是 ImGui 屏幕坐标，需要设计尺寸时用 `ui::S(v)` 换算。
* **颜色**：`Color` 是直通 alpha 的 sRGB 浮点色；很多参数用 `Color::Clear()`（alpha = 0）表示“用默认值”。
* 所有接口都在命名空间 `wgt`（控件在 `wgt::ui`，图标在 `wgt::icons`）。只需 `#include <wgt/wgt.hpp>`。

---

## 目录

1. [头文件一览](#1-头文件一览)
2. [基础类型（math.hpp / config.hpp）](#2-基础类型)
3. [Context：生命周期与服务（context.hpp）](#3-context)
4. [控件（ui.hpp）](#4-控件)
5. [逐组件样式 ItemStyle](#5-逐组件样式-itemstyle)
6. [自动布局](#6-自动布局)
7. [自定义控件](#7-自定义控件)
8. [Painter / Style / Paint（painter.hpp）](#8-painter)
9. [主题（theme.hpp）](#9-主题)
10. [文字（text.hpp）](#10-文字)
11. [动画（anim.hpp）](#11-动画)
12. [线程工具与回调（sync.hpp / function.hpp）](#12-线程工具与回调)
13. [帧率控制 FramePacer（pacing.hpp）](#13-framepacer)
14. [自定义 HLSL 效果](#14-自定义-hlsl-效果)
15. [插件](#15-插件)
16. [自定义渲染后端（backend.hpp / fx.hpp）](#16-自定义渲染后端)
17. [图标（icons.hpp）](#17-图标)
18. [版本与 ABI](#18-版本与-abi)

---

## 1. 头文件一览

| 头文件 | 内容 |
| --- | --- |
| `wgt/wgt.hpp` | 总头文件，包含下面全部 |
| `wgt/config.hpp` | 导出宏 `WGT_API`、版本号、`WGT_ABI_VERSION`、`EffectId` / `Icon` 类型 |
| `wgt/math.hpp` | `Vec2`、`Rect`、`Color` 与数学函数 |
| `wgt/function.hpp` | `Callback<>` 跨 DLL 回调、`Task` |
| `wgt/sync.hpp` | `Property<T>`、`Channel<T>`、`Latest<T>` 线程安全数据 |
| `wgt/text.hpp` | 字体注册、`FontRef`、`MeasureText` |
| `wgt/theme.hpp` | 主题令牌：`Theme`、`Palette`、`GlassMaterial`、`Spring`、`GlassLook` |
| `wgt/anim.hpp` | 弹簧动画 `anim::` |
| `wgt/fx.hpp` | Painter 与渲染后端之间的 FX 命令格式 |
| `wgt/painter.hpp` | `Painter`、`Style`、`Paint` |
| `wgt/context.hpp` | `Context`、`ContextDesc`、`RenderTarget`、通知、面板、插件 |
| `wgt/pacing.hpp` | `FramePacer` |
| `wgt/backend.hpp` | `IRenderBackend` 自定义后端接口 |
| `wgt/icons.hpp` | 图标常量 |
| `wgt/ui.hpp` | 全部控件、逐组件样式、自动布局 |

---

## 2. 基础类型

### Vec2 / 数学函数

`Vec2` 就是 `ImVec2`。

| 函数 | 说明 |
| --- | --- |
| `Clamp(v, lo, hi)` / `Saturate(v)` | 限制到区间 / 到 0..1 |
| `Lerp(a, b, t)` | 线性插值（float、Vec2、Color 都有） |
| `Remap(v, a0, a1, b0, b1)` | 区间映射 |
| `SmoothStep(e0, e1, v)` | 平滑阶跃 |
| `Length(v)` | 向量长度 |
| `Radians(deg)` / `Degrees(rad)` | 角度换算；常量 `kPi`、`kTau` |

### Rect

轴对齐矩形（`min` 含、`max` 不含），ImGui 坐标。

| 成员 | 说明 |
| --- | --- |
| `Rect(x0, y0, x1, y1)` / `Rect(min, max)` | 构造 |
| `Rect::FromSize(pos, size)` / `Rect::FromCenter(c, size)` | 由位置+尺寸 / 中心+尺寸构造 |
| `Width()` `Height()` `Size()` `Center()` | 尺寸与中心 |
| `Contains(p)` `Overlaps(r)` `Empty()` | 判断 |
| `Expanded(a)` / `Expanded(ax, ay)` / `Shrunk(a)` | 外扩 / 内缩 |
| `Translated(d)` `Scaled(s)` | 平移 / 以中心缩放 |
| `Intersect(r)` `Union(r)` | 交集 / 并集 |
| `Left(w)` `Right(w)` `Top(h)` `Bottom(h)` | 切出一条 |

### Color

| 成员 | 说明 |
| --- | --- |
| `Color(r, g, b, a = 1)` | 0..1 浮点 |
| `Color::Hex(0x007AFF, alpha = 1)` | 十六进制 |
| `Color::Rgba8(r, g, b, a = 255)` | 0..255 整数 |
| `Color::White(a)` `Color::Black(a)` `Color::Clear()` | 常用色（`Clear` = 全透明，常用作“默认值”） |
| `Color::Hsv(h, s, v, a)` | HSV |
| `WithAlpha(a)` `Fade(k)` | 设置 alpha / alpha 乘 k |
| `Lighter(k)` `Darker(k)` | 向白 / 向黑混合 |
| `Luminance()` `IsVisible()` | 亮度 / alpha 是否大于 0 |
| `ToVec4()` `ToU32()` | 转 ImGui 类型 |

### 其他

| 类型 / 函数 | 说明 |
| --- | --- |
| `EffectId` | `Context::RegisterEffect` 返回的效果句柄，0 = 无 |
| `Icon` | 图标字形的 Unicode 码位（见第 17 节） |
| `GetVersionString()` / `GetAbiVersion()` | 运行时版本字符串 / DLL 的 ABI 版本 |

---

## 3. Context

### 3.1 ContextDesc（创建参数）

| 字段 | 默认 | 说明 |
| --- | --- | --- |
| `backend` | `D3D11` | `D3D11` / `D3D12` / `Custom` |
| `hwnd` | null | 用于输入、DPI、光标的窗口 |
| `d3d11Device` / `d3d11Context` | null | DX11 设备与立即上下文 |
| `d3d11RestoreState` | true | `Render()` 前后备份 / 还原宿主的管线状态 |
| `d3d12Device` | null | DX12 设备 |
| `d3d12FramesInFlight` | 3 | 必须 ≥ 宿主的帧延迟 |
| `customBackend` | null | 自定义后端（不接管所有权，必须比 Context 活得久） |
| `darkMode` | true | 深色 / 浅色 |
| `accent` | Clear | 强调色，Clear = 系统蓝 |
| `uiScale` | 1 | 用户缩放倍数 |
| `dpiScale` | 0 | 0 = 跟随 `hwnd` 所在显示器（逐显示器 DPI），> 0 = 固定 |
| `renderScale` | 0 | 0 = 自动（渲染目标尺寸 / 窗口客户区尺寸），> 0 = 固定 |
| `fontFamily` | null | 界面字体族名，null = Segoe UI Variable（Win10 为 Segoe UI） |
| `fontFamilyDisplay` | null | 大字号用的字体族（可选） |
| `monoFamily` | null | 等宽字体，null = Cascadia Mono → Consolas |
| `iconFamily` | null | 图标字体，null = Segoe Fluent Icons → Segoe MDL2 Assets |
| `fontFiles` / `fontFileCount` | null / 0 | 额外注册的 .ttf/.otf/.ttc 文件 |
| `locale` | null | BCP-47 语言标签（如 `"zh-CN"`），影响回退与 CJK 字形 |
| `textAntialiasing` | `Auto` | `Auto` / `Grayscale` / `Subpixel` |
| `textGamma` | 0 | > 0 覆盖系统文字 gamma（1.0..2.2） |
| `textContrast` | -1 | ≥ 0 覆盖系统增强对比度（0..2） |
| `iniFilename` | null | ImGui 窗口布局持久化文件，null = 关闭 |
| `toggleKey` | 0 | 显示 / 隐藏界面的虚拟键码，0 = 无 |
| `showDock` | true | 显示面板启动栏（Dock） |
| `fixedDeltaTime` | 0 | > 0 时动画每帧固定前进该步长（测试、录屏） |
| `debugLayout` | false | 布局检查器：红框标出重叠控件，橙框标出被窗口右边缘截断的控件，并写日志 |
| `maxBackdropCaptures` | 64 | 每帧玻璃背景采样上限，超出后沿用上一次采样并警告一次 |

### 3.2 RenderTarget

| 成员 | 说明 |
| --- | --- |
| `RenderTarget::D3D11(rtv)` | DX11：渲染到这个 RTV |
| `RenderTarget::D3D12(cmdList, resource, rtvHandle, rtvFormat = 0)` | DX12：资源需处于 `RENDER_TARGET` 状态，调用后保持该状态；`rtvFormat` 0 = 资源格式 |

### 3.3 生命周期（UI 线程）

| 方法 | 说明 |
| --- | --- |
| `static Context* Create(const ContextDesc&)` | 创建并设为当前线程的上下文（该线程成为 UI 线程） |
| `void Destroy()` | 销毁 |
| `void NewFrame()` | 开始一帧（之后可以调用控件） |
| `void Render(const RenderTarget&)` | 结束并渲染一帧 |
| `void InvalidateDeviceObjects()` | 立即释放与渲染目标同尺寸的 GPU 资源（按需重建）；改尺寸不需要调用；设备丢失需重建 Context |

### 3.4 服务 `[任意线程]`

| 方法 | 说明 |
| --- | --- |
| `bool HandleWin32Message(hwnd, msg, wparam, lparam)` | 转发 Win32 消息；返回 true 表示界面需要吞掉该消息 |
| `bool WantsMouse()` / `bool WantsKeyboard()` | 界面当前是否占用鼠标 / 键盘 |
| `SetVisible(bool)` / `IsVisible()` / `ToggleVisible()` | 显示 / 隐藏整个界面 |
| `void Post(Task task)` | 在下一帧开始时于 UI 线程执行 `task` |
| `SetTheme(const Theme&, bool animate = true)` | 替换主题（带过渡动画） |
| `SetDarkMode(bool, bool animate = true)` / `IsDarkMode()` | 深浅色 |
| `SetAccent(Color, bool animate = true)` | 强调色 |
| `SetGlassLook(GlassLook)` / `GetGlassLook()` | 整个界面的默认玻璃观感（默认 `Frosted`） |
| `SetUiScale(float)` | 用户缩放，1 = 默认 |
| `SetDpiScale(float)` | 0 = 跟随显示器 |
| `SetRenderScale(float)` | 0 = 自动 |
| `ScaleInfo GetScaleInfo()` | 当前缩放信息 |
| `Notify(const Notification&)` | 发通知（灵动岛：先展开为完整卡片，再变成胶囊驻留） |
| `SetActivity(id, title, progress, icon = 0, tint = Clear)` | 实时活动（灵动岛进度），`progress < 0` = 不确定进度 |
| `ClearActivity(id)` | 结束实时活动 |
| `bool AddPanel(const PanelDesc&, PanelFn draw)` | 注册面板（出现在 Dock，WGT 管窗口） |
| `RemovePanel(id)` / `SetPanelOpen(id, bool)` / `IsPanelOpen(id)` | 面板管理 |
| `SetDockVisible(bool)` | 显示 / 隐藏 Dock |
| `EffectId RegisterEffect(name, hlslSource)` | 注册自定义 HLSL 效果（后台编译约 1 秒，见第 14 节） |
| `AddFontFile(const wchar_t* path)` | 注册字体文件（下一帧生效） |
| `ImTextureID CreateTexture(rgba8, w, h)` / `DestroyTexture(tex)` | RGBA8 贴图 |
| `bool LoadPlugin(dllPath)` / `int LoadPluginsFromDirectory(dir)` / `AddPlugin(IPlugin*)` | 插件（见第 15 节） |
| `SetLogCallback(LogFn)` | 日志回调 `void(int level, const char* message)` |
| `InjectMousePos(x, y)` / `InjectMouseButton(button, down)` / `InjectMouseWheel(y, x = 0)` / `InjectKey(key, down)` / `InjectText(utf8)` / `EndInputInjection()` | 注入输入（自动化 / UI 测试），下一帧生效 |
| `SetDebugLayout(bool)` | 开关布局检查器 |
| `SetTextAntialiasing(mode)` / `GetTextAntialiasing()` / `IsSubpixelTextActive()` | 文字抗锯齿 |
| `FrameStats GetStats()` | 帧统计 |
| `Backend GetBackend()` / `ImGuiContext* GetImGuiContext()` | 后端类型 / ImGui 上下文 |

UI 线程专用：`TextRenderingInfo GetTextRenderingInfo()`（gamma、对比度、ClearType 状态），`const Theme& GetTheme()`（当前带动画的主题）。

### 3.5 相关结构

**Notification**：`title`、`message`、`icon`、`tint`（Clear = 强调色）、`duration`（秒，默认 3.5）。

**PanelDesc**：`id`（唯一且稳定）、`title`（默认同 id）、`icon`、`iconColor`、`size`（默认 420×540）、`pos`（-1 = 自动）、`open`、`flags`。

**PanelFlags_**：`HideFromDock`、`NoClose`、`NoResize`、`Solid`（不透明表面）、`NoScroll`（面板自己管理滚动，如导航）、`ClearGlass`（窗口表面全透明）。

**ScaleInfo**：`dpi`、`user`、`metrics`（= dpi × user，所有尺寸乘这个）、`render`（每 UI 单位的渲染目标像素）。

**FrameStats**：`fps`、`frameMs`、`cpuUiMs`（NewFrame→Render 的 CPU 耗时）、`drawCalls`、`fxInstances`、`backdropCaptures`、`glowLayers`、`vertices`、`gpuMs`、`gpuGlassMs`（背景采样与模糊金字塔）、`gpuLayerMs`（发光层）。GPU 时间来自时间戳查询，会晚几帧，不可用时为 0。

### 3.6 全局函数

| 函数 | 说明 |
| --- | --- |
| `Context* GetCurrentContext()` / `SetCurrentContext(ctx)` | 当前线程的上下文 |
| `const Theme& CurrentTheme()` | 当前上下文的主题（UI 线程） |
| `FontRef GetFont(TextStyle)` | 主题文字样式对应的字体（已按缩放计算字号） |
| `FontRef GetFont(FontWeight, float unscaledSize)` | 指定字重、设计字号 |
| `ImFont* GetImGuiFont(FontWeight)` | 给原生 ImGui 控件用的 DirectWrite 字体 |

---

## 4. 控件

所有控件都在 `wgt::ui`，只能在 UI 线程、`NewFrame()` 与 `Render()` 之间调用，可以和原生 ImGui 调用混用。任意控件都可以用 `ui::Next()` 单独改样式（第 5 节）。

### 4.1 文字

文字比所在行剩余宽度还宽时，会在容器边缘自动换行（跟在 `SameLine` 后面的也一样）；自动布局容器里由容器决定尺寸。

| 函数 | 说明 |
| --- | --- |
| `Text(TextStyle, fmt, ...)` | 格式化文字 |
| `TextColored(TextStyle, Color, fmt, ...)` | 带颜色 |
| `TextSecondary(fmt, ...)` | 次要文字（Subheadline + 次要标签色） |
| `TextWrapped(TextStyle, Color, text)` | 总是换行 |
| `LargeTitle(text)` / `Headline(text)` | 大标题 / 标题 |
| `Spacer(height = -1)` | 竖向间距，< 0 = 主题间距 |
| `Divider()` | 分隔线 |

### 4.2 按钮

```cpp
bool Button(const char* label, const ButtonOptions& options = {});
bool IconButton(const char* id, Icon icon, const ButtonOptions& options = {});
```
返回 true = 本帧被点击。

**ButtonOptions**

| 字段 | 默认 | 说明 |
| --- | --- | --- |
| `kind` | `Filled` | `Filled`（强调色底白字）、`Tinted`（半透明强调色）、`Gray`、`Plain`（只有文字）、`Glass`（液态玻璃）、`GlassProminent`（带强调色的玻璃）、`Destructive`（红） |
| `size` | `Regular` | `Small` / `Regular` / `Large` |
| `icon` | 0 | 文字前的图标 |
| `tint` | Clear | Clear = 强调色（Destructive 为红） |
| `width` | 0 | 0 = 适应内容，< 0 = 填满可用宽度，> 0 = 固定 |
| `capsule` | true | 胶囊 / 圆角矩形 |
| `glow` | false | 霓虹光晕 |

### 4.3 开关、勾选、滑块、步进器

| 函数 | 说明 |
| --- | --- |
| `bool Toggle(id, bool* value)` | iOS 开关 |
| `bool Checkbox(label, bool* value)` | 圆形勾选 |
| `bool ToggleButton(id, bool* value, Icon, const ToggleButtonOptions& = {})` | 控制中心圆形玻璃开关 |
| `bool Slider(id, float* value, min, max, const SliderOptions& = {})` | 液态滑块（超出范围的值显示在最近的一端，拖动后写回范围内的值） |
| `bool Stepper(id, int* value, min, max, step = 1)` | 步进器 |

返回 true = 值被改变。`Toggle`、`Slider`、`Stepper` 还有接受 `Property<T>&` 的重载（读 → 编辑 → 改了才写回，线程安全）。

**ToggleButtonOptions**：`tint`（“开”的颜色，Clear = 强调色）、`diameter`（默认 58）、`glow`（默认 true，开启时柔和光晕）。

**SliderOptions**：`width`（< 0 = 填满）、`minIcon` / `maxIcon`、`tint`、`step`（吸附步长，0 = 连续）。

### 4.4 分段控件、选择器、输入框

| 函数 | 说明 |
| --- | --- |
| `bool Segmented(id, int* selected, const char* const* items, int count, float width = -1)` | 分段控件（选中项以液态透镜移动），`width < 0` = 填满 |
| `bool Picker(id, int* selected, items, count, float width = 0)` | 下拉选择器，`width` 0 = 适应最长项，< 0 = 填满 |
| `bool TextField(id, char* buffer, size, placeholder = null, const TextFieldOptions& = {})` | 文本框（WGT 自己的编辑器，按字素簇移动，内联输入法） |
| `bool SearchField(id, buffer, size, placeholder = "Search")` | 带搜索图标的文本框 |

返回 true = 选择或文字改变。

**TextFieldOptions**：`icon`、`width`（< 0 = 填满）、`password`、`clearButton`（默认 true）、`background`（默认 true；false = 无底色，放在玻璃或自定义表面上）。

### 4.5 进度、图表、状态

| 函数 | 说明 |
| --- | --- |
| `ProgressBar(fraction, const ProgressOptions&)` | 进度条 |
| `ProgressRing(fraction, const ProgressOptions&)` | 进度环 |
| `ProgressBar(fraction, width = -1, tint = Clear)` / `ProgressRing(fraction, diameter = 0, thickness = 0, tint = Clear)` | 旧写法，同样默认不发光 |
| `LineChart(id, const float* values, int count, const LineChartOptions& = {})` | 折线图 |
| `ActivityIndicator(diameter = 0, tint = Clear)` | 转圈（diameter 0 = 24） |
| `Badge(text, tint = Clear)` | 徽章（Clear = 红） |
| `Image(tex, size, radius = -1, uv0, uv1)` | 圆角图片，radius < 0 = 主题控件圆角 |
| `bool ColorSwatches(id, int* selected, const Color* colors, count, diameter = 0)` | 色块选择 |
| `Tooltip(text)` | 给上一个控件加玻璃气泡提示 |

**ProgressOptions**：`tint`、`glow`（默认 false）、`width`（进度条，< 0 = 填满）、`diameter`（进度环，0 = 44）、`thickness`（进度环，0 = 按直径比例）。

**LineChartOptions**

| 字段 | 默认 | 说明 |
| --- | --- | --- |
| `height` / `width` | 90 / -1 | 尺寸，`width < 0` = 填满 |
| `rect` | 空 | 非空时画进这个矩形（行、卡片里），不占布局 |
| `tint` | Clear | 线色，Clear = 强调色 |
| `thickness` | 2 | 线宽 |
| `smooth` | true | 平滑曲线（单调，不越过数据点）；false = 折线 |
| `fill` | true | 线下渐变 |
| `glow` | false | 整条线一个均匀光晕 |
| `lastPoint` | true | 最新值处的圆点 |
| `min` / `max` | 0 / 0 | 数值范围；相等 = 按数据自动拟合（正数据从 0 开始），变化时缓动 |
| `offset` | 0 | 环形缓冲区中最旧一项的下标 |

### 4.6 窗口

```cpp
bool BeginWindow(const char* title, bool* open = nullptr, const WindowOptions& options = {});
void EndWindow();   // BeginWindow 返回 true 时必须调用
```

**WindowOptions**：`size`（默认 420×540）、`pos`（-1 = 居中错位）、`flags`、`subtitle`、`icon`。

**WindowFlags_**：`NoClose`、`NoResize`、`NoMove`、`NoHeader`、`NoScroll`、`Solid`（不透明）、`LargeTitle`（内容区大标题）、`NoShadow`、`NoPadding`、`ClearGlass`（窗口表面全透明）。

### 4.7 分组列表（iOS 设置风格）

| 函数 | 说明 |
| --- | --- |
| `bool BeginSection(header = null, footer = null)` / `EndSection()` | 分组；返回 true 时才调用 `EndSection` |
| `bool RowNavigation(label, detail = null, RowIcon = {})` | 带箭头的导航行，点击返回 true |
| `bool RowToggle(label, bool* value, RowIcon = {})` | 开关行 |
| `bool RowSlider(label, float* value, min, max, RowIcon = {}, format = "%.0f")` | 滑块行：超出范围的值显示在最近的一端，数字也显示夹到范围内的值（数据本身在拖动前不改）；数值列按 min / max 中较宽的一个定宽，滑块长度不随数值跳动；窄时数值移到标签那一行 |
| `bool RowStepper(label, int* value, min, max, RowIcon = {})` | 步进器行 |
| `bool RowSegmented(label, int* selected, items, count, RowIcon = {})` | 分段控件行 |
| `bool RowPicker(label, int* selected, items, count, RowIcon = {})` | 选择器行 |
| `RowValue(label, value, RowIcon = {})` | 只显示值 |
| `bool RowButton(label, destructive = false, RowIcon = {})` | 按钮行 |
| `bool BeginRow(id, height = 0, Rect* outContent = null)` / `EndRow()` | 自定义行：返回内容矩形，画完调用 `EndRow`（无论返回值）；其中填满宽度的控件（滑块、输入框、`width = -1` 的按钮等）不会超出这一行 |

`RowToggle` / `RowSlider` 也有 `Property<T>&` 重载。**RowIcon**：`icon`、`color`（图标块颜色，Clear = 强调色）。

### 4.8 卡片

```cpp
bool BeginCard(const char* id, Vec2 size = Vec2(0, 0), const CardOptions& options = {});
void EndCard();
```
`size.x` 0 = 填满宽度，< 0 = 填满后减去该值；`size.y` 0 = 按内容。卡片里填满宽度的控件不会超出卡片的内边距。**CardOptions**：`glass`（玻璃卡片）、`radius`（< 0 = 主题）、`fill`（Clear = 主题卡片色）、`shadow`（默认 true）、`padding`（< 0 = 主题）。

### 4.9 导航、标签栏、搜索栏、滚动区域

| 函数 | 说明 |
| --- | --- |
| `bool BeginNavigation(id, rootPage)` / `EndNavigation()` | 栈式导航（iOS 推入 / 返回动画与返回按钮） |
| `bool BeginPage(pageId, title)` / `EndPage()` | 页面 |
| `NavigationPush(pageId)` / `NavigationPop()` | 推入 / 返回 |
| `bool TabBar(id, int* selected, const TabItem* items, count)` | 悬浮玻璃标签栏，选择改变返回 true。**TabItem**：`label`、`icon` |
| `bool SearchBar(id, buffer, size, const SearchBarOptions&)` / `SearchBar(id, buffer, size, placeholder)` | 固定在页面底部的悬浮搜索栏，内容从下面滚过；文字改变返回 true |
| `bool BeginScrollArea(id, Vec2 size = 0)` / `EndScrollArea()` | 平滑滚动区域（自动隐藏的滚动条、拖动滚动、边缘渐隐）；`size` 0 = 占满剩余空间 |

```cpp
if (ui::BeginNavigation("settings", "root")) {
    if (ui::BeginPage("root", "设置")) { if (ui::RowNavigation("通用")) ui::NavigationPush("general"); ui::EndPage(); }
    if (ui::BeginPage("general", "通用")) { /* ... */ ui::EndPage(); }
    ui::EndNavigation();
}
```

**SearchBarOptions**：`placeholder`、`look`（栏的玻璃：默认 `Clear`）、`base`（输入框内的柔和底色，默认 true）、`fill`（底色颜色，Clear = 主题色）。对搜索栏使用 `Clear` 样式时整条栏全透明，底色也去掉。

---

## 5. 逐组件样式 ItemStyle

不改主题、不用成对 Push/Pop，就能单独修改任意组件。只改你设置的项，其余跟随主题（以及外层作用域，逐项以最内层为准）。

```cpp
ui::Next().Tint(Color::Hex(0x30D158)).Radius(8);                   // 只作用于下一个组件
ui::Button("开始游戏");

ui::Next().Look(GlassLook::Frosted).Blur(24).Fill(purple);         // 实心按钮变成紫色雾面玻璃
ui::Button("雾面");

ui::Next().Look(GlassLook::Clear).Refraction(20).Dispersion(0.9f); // 全透明 + 强透镜
ui::Slider("vol", &vol, 0, 1);

ui::Next().Fill(blue).SelectedFill(Color::White(0.92f)).MovingFill(Color::White(0.25f))
          .Label(Color::White()).SelectedLabel(blue);               // 分段控件各部分的颜色
ui::Segmented("range", &range, items, 3);
```

### 5.1 可设置的项

| 修饰 | 作用 |
| --- | --- |
| `Look(GlassLook)` | `Theme`（主题材质）、`Clear`（全透明）、`Frosted`（iOS 雾面） |
| `Blur(v)` | 磨砂模糊半径 |
| `Refraction(v)` | 折射（透镜）强度 |
| `Bezel(v)` | 弧形边缘宽度 |
| `Dispersion(v)` | 边缘色散（0..1） |
| `Saturation(v)` / `Brightness(v)` | 背景饱和度 / 提亮 |
| `Specular(v)` | 边缘高光 |
| `Legibility(v)` | 可读性（按背景明暗压暗 / 提亮） |
| `Magnify(v)` | 放大镜效果 |
| `GlassTint(Color)` | 玻璃着色（alpha = 着色量） |
| `Rim(Color)` | 发丝线边框 |
| `Glass(GlassMaterial)` | 整个玻璃材质 |
| `Tint(Color)` | 强调色：实心按钮、开关“开”、滑块、进度、勾选、光标、选中标签 |
| `Fill(Color)` | 底色：按钮、输入框、轨道、分段控件、分组 / 卡片 / 窗口背景、徽章 |
| `Radius(float)` | 圆角（UI 单位） |
| `Opacity(float)` | 整个组件的不透明度（包括其中的文字和原生 ImGui 控件） |
| `Label(Color)` | 文字和符号颜色 |
| `SelectedFill(Color)` | 分段控件 / 标签栏：切换完成后选中项的背景 |
| `MovingFill(Color)` | 分段控件 / 标签栏：切换过程中移动的液态透镜颜色（alpha = 着色强度） |
| `SelectedLabel(Color)` | 分段控件 / 标签栏：选中项文字（选中底落定时切换过去） |

规则：

* **实心组件变玻璃**：对实心按钮、开关轨道、输入框、卡片等设置 `Look(Clear)`、`Look(Frosted)` 或任意玻璃参数，它的表面就变成那种玻璃，原来的颜色作为玻璃的着色。`Theme` 保持实心。
* **全透明（Clear）**：所有表面变成透明玻璃（透镜、边缘光、发丝线）；开关的“开”、选中项、进度等状态色保留但半透明；原本放在实心底色上的文字改用标签色，任何背景上都清晰。
* **分段控件**：只设了 `Fill` 而没设 `SelectedFill` 时，选中项默认是一块浅色半透明底，在任何颜色上都协调。
* **优先级**：控件自己的选项（如 `ButtonOptions::tint`、`CardOptions::radius`、`CardOptions::fill`、`SearchBarOptions::fill`、`Badge` 的颜色参数）优先于样式。
* **容器**：放在 `BeginWindow` / `BeginSection` / `BeginCard` / `BeginRow` 前，作用于该容器及其中所有组件。

### 5.2 函数

| 函数 | 说明 |
| --- | --- |
| `ItemStyle& Next()` | 下一个组件的样式（原地修改，链式调用） |
| `SetNextItemStyle(const ItemStyle&)` | 整个替换下一个组件的样式 |
| `PushItemStyle(const ItemStyle&)` / `PopItemStyle()` | 作用域（可嵌套） |
| `StyleScope s(ItemStyle().Tint(red));` | RAII 作用域，析构时自动 Pop |
| `ItemStyle CurrentItemStyle()` | 当前生效的样式（所有作用域合并后） |
| `Color AccentColor()` | 当前样式的强调色，没设则为主题强调色 |
| `SetNextItemLook(look)` / `PushGlassLook(look)` / `PopGlassLook()` | 只改观感的简写 |
| `GlassLook CurrentGlassLook()` | 当前观感 |
| `GlassMaterial LookMaterial(const GlassMaterial& themed)` | 主题材质经当前样式处理后的结果（自定义控件用） |
| `Context::SetGlassLook(look)` | 整个界面的默认观感（默认 `Frosted`，任意线程） |

`ItemStyle` 的数据成员：`set`（已设置项的位掩码，`Has(ItemStyle::kTint)` 之类可查询）、`look`、`glass`、`tint`、`fill`、`label`、`selectedFill`、`movingFill`、`selectedLabel`、`radius`、`opacity`。

---

## 6. 自动布局

容器按可用空间测量并摆放子项，窗口大小变化时带弹簧动画重排。直接放在容器里的每个控件（或 `ImGui::BeginGroup/EndGroup` 块、嵌套容器）是一个子项；拉伸到可用宽度的控件（滑块、输入框、`width = -1` 的按钮等）自动成为弹性子项；发光的子项会自动获得额外间距。

| 函数 | 说明 |
| --- | --- |
| `BeginVStack(id, const StackOptions& = {})` / `BeginHStack(...)` / `EndStack()` | 竖向 / 横向堆叠 |
| `BeginAdaptiveStack(id, ...)` / `EndStack()` | 放得下就横排，否则竖排（SwiftUI ViewThatFits） |
| `BeginGrid(id, const GridOptions& = {})` / `EndGrid()` | 网格（自适应列数） |
| `BeginFlow(id, const FlowOptions& = {})` / `EndFlow()` | 像文字一样换行 |
| `LayoutFlex(float)` | 下一个子项占剩余主轴空间的比例（堆叠） |
| `LayoutSpan(int)` | 下一个子项跨几列（网格） |
| `FlexSpacer()` | 弹性空白（在 HStack 里把两边推开） |
| `SizeClass GetSizeClass(width = -1)` | 宽度等级：`Compact`（< 420）、`Regular`（< 760）、`Expanded`；默认取光标处可用宽度 |

Begin 返回 true 时才调用对应的 End。

**StackOptions**：`spacing`（< 0 = 主题）、`align`（交叉轴对齐，默认 Center）、`justify`（无弹性子项时主轴分布，默认 Start）、`minFillWidth`（自适应堆叠中弹性子项的最小宽度，默认 120）、`animate`。

**GridOptions**：`minColumnWidth`（默认 160，能放几列放几列）、`columns`（> 0 = 固定列数）、`maxColumns`、`spacing`、`rowSpacing`、`cellHeight`（> 0 = 固定行高）、`aspect`（> 0 = 宽高比）、`align`（默认 Stretch）、`animate`。

**FlowOptions**：`spacing`、`lineSpacing`、`justify`（每行）、`align`（行内竖向对齐）、`animate`。

**Align**：`Start`、`Center`、`End`、`Stretch`（交叉轴 = 填满；主轴 = 两端分布）。

```cpp
ui::GridOptions g; g.minColumnWidth = 150;
if (ui::BeginGrid("tiles", g)) { for (auto& it : items) Tile(it); ui::EndGrid(); }
```

---

## 7. 自定义控件

| 函数 | 说明 |
| --- | --- |
| `Interaction Interact(id, Vec2 size, flags = 0)` | 占用布局空间并处理按下 / 悬停，状态带弹簧动画 |
| `Interaction InteractRect(ImGuiID id, const Rect&, flags = 0)` | 同上，用给定矩形（不占布局） |
| `float S(float v)` | 按主题缩放：`S(12) == 12 * metrics.scale` |
| `float AvailableWidth()` | 光标处可用宽度；在自动布局容器里调用会让该控件成为弹性子项 |

**Interaction**：`id`、`rect`、`visible`、`hovered`、`held`、`pressed`（本帧点击）、`hover` / `press`（0..1 弹簧值）、`style`（它接收到的 `ui::Next()` 样式）。

**InteractFlags_**：`NoLayout`（仅 InteractRect）、`PressOnClick`（按下即触发）、`AllowOverlap`、`Repeat`（按住重复）。

```cpp
ui::Interaction it = ui::Interact("card", Vec2(ui::AvailableWidth(), ui::S(120)));
if (it.visible)
{
    ui::StyleScope s(it.style);   // 让 LookMaterial / AccentColor 跟随 ui::Next() 设置的样式
    Painter p;
    p.Rect(it.rect, Style().Radius(ui::S(20)).Glass(ui::LookMaterial(CurrentTheme().materials.control)));
    if (it.pressed) OpenCard();
}
```

---

## 8. Painter

`Painter` 是所有 WGT 控件背后的绘制类：SDF 形状 + 材质（渐变、描边、柔和阴影、内阴影、内外发光、液态玻璃、图片填充、微光、圆角遮罩、辉光层），分辨率无关，GPU 解析式抗锯齿。每个形状一个 GPU 实例，开销不随尺寸和效果数量增长。

### 8.1 构造与形状

| 方法 | 说明 |
| --- | --- |
| `Painter()` / `Painter(ImDrawList*)` | 画到当前 ImGui 窗口 / 指定绘制列表 |
| `Rect(r, style)` | 圆角矩形（`Style::Radius`，连续曲率圆角） |
| `Capsule(r, style)` | 胶囊 |
| `Circle(center, radius, style)` | 圆 |
| `Arc(center, radius, thickness, startRad, sweepRad, style)` | 圆弧（圆头，弧度，顺时针，0 = +x） |
| `Ring(center, radius, thickness, style)` | 圆环 |
| `Line(a, b, thickness, style)` | 圆头线段 |
| `Merge(a, b, radius, smoothness, style)` | 两个圆角矩形的液态融合（metaball） |
| `Polyline(points, count, thickness, style, flags = 0)` | 一笔画完的折线（斜接、急转弯截断、圆头、像素级抗锯齿）；`style` 有 Glow 才发光且整条一个光晕；`PolylineFlags_Smooth` = 平滑曲线 |
| `Area(points, count, baseline, paint, flags = 0)` | 路径与 `y = baseline` 之间的填充（面积图） |
| `FillRect(r, color, rounding = 0)` / `HLine(x0, x1, y, color, thickness = 1)` | 廉价图元（HLine 对齐物理像素） |
| `Image(tex, r, radius, tint = White, uv0, uv1)` | 圆角图片 |

### 8.2 文字与图标

| 方法 | 说明 |
| --- | --- |
| `Vec2 Text(pos, TextStyle, color, text, end = null, wrapWidth = 0)` | 左上角为 `pos` |
| `Vec2 Text(pos, FontRef, color, text, end = null, wrapWidth = 0, flags = 0)` | 指定字体 |
| `Vec2 TextBox(r, align, FontRef, color, text, end = null, flags = 0)` | 在矩形内对齐（(0,0) 左上，(0.5,0.5) 居中）；`TextFlags_Ellipsis` 超宽加省略号 |
| `Vec2 TextAligned(r, align, TextStyle, color, text, end = null)` | 同上，用文字样式 |
| `static Vec2 MeasureText(TextStyle / FontRef, text, end = null, wrapWidth = 0)` | 测量 |
| `Icon(center, icon, size, color)` | 图标（光学居中，`size` = em 尺寸） |
| `static float SnapToPixel(v)` | 对齐到物理像素 |

### 8.3 状态与特效

| 方法 | 说明 |
| --- | --- |
| `PushMask(r, radius)` / `PopMask()` | 圆角遮罩（最多 8 层，最内层生效） |
| `PushClip(r, intersect = true)` / `PopClip()` | 裁剪 |
| `PushScale(origin, scale)` / `PopScale()` | 以 origin 为中心统一缩放其间的形状与文字 |
| `SetAlpha(a)` / `Alpha()` | 整体透明度 |
| `BeginGlowLayer(tint, radius, intensity = 1, contentOpacity = 1)` / `EndGlowLayer()` | 其间所有内容加 GPU 辉光（霓虹文字、图标、线条）；`tint.a` = 光晕向 tint 着色的程度 |
| `BeginEdgeFade(region, top, bottom)` / `EndEdgeFade()` | 其间内容在 region 上 / 下边缘渐隐（真实透明，不加颜色） |
| `LightStreak(r, intensity, thickness, speed = 1, smile = 0, open = 0.30, time = -1)` | Siri 光球的光线：几条柔化细线，两端收拢、中间绽开，顶部色散成暖橙 / 淡绿 / 淡紫细线，下方白线与白光；在上面盖透明玻璃即被折射。`open` = 中间张开幅度，`time` = 动画自己的时钟（< 0 用帧时钟） |
| `Emit(const fx::Instance&, effect = 0, texture = none)` | 低层：直接提交实例 |
| `DrawList()` | 当前绘制列表 |

### 8.4 Style（链式材质描述）

| 方法 | 说明 |
| --- | --- |
| `Fill(Color)` / `Fill(const Paint&)` | 填充 |
| `Radius(r)` / `Radius(tl, tr, br, bl)` | 圆角 |
| `Smoothing(s)` | 圆角平滑度（0 = 圆弧，1 = 连续曲率），< 0 = 主题 |
| `Stroke(width, color, align = 0)` | 描边（0 内、0.5 居中、1 外） |
| `StrokeFade(fadeTo, angleDeg = 90)` | 描边沿方向渐隐 |
| `Shadow(color, blur, offset = 0, spread = 0)` | 外阴影 |
| `InnerShadow(color, blur, offset = 0, spread = 0)` | 内阴影 |
| `Glow(color, radius, intensity = 1)` | 外发光 |
| `InnerGlow(radius, intensity = 1)` | 内发光 |
| `ContainGlow(bool)` | 默认 true：光晕在碰到相邻控件前渐隐并在布局中预留空间；false = 可溢出（装饰背景） |
| `Glass(const GlassMaterial&)` | 液态玻璃 |
| `Image(tex, uvMin, uvMax)` | 图片填充 |
| `Opacity(o)` | 不透明度 |
| `Shimmer(intensity, speed = 0.55)` | 骨架屏微光 |
| `Noise(n)` | 玻璃颗粒（< 0 = 默认） |
| `Effect(id, p0, p1, p2, p3)` | 使用自定义 HLSL 效果（参数进入 `fx.params`） |

### 8.5 Paint（填充）

| 工厂 | 说明 |
| --- | --- |
| `Paint::Solid(c)` | 纯色 |
| `Paint::Linear(from, to, angleDeg = 90)` | 线性渐变（0 = 左→右，90 = 上→下） |
| `Paint::Radial(inner, outer, center = (0.5,0.5), radius = 0.5)` | 径向渐变（center 为边界内比例，radius 为最长边比例） |
| `Paint::Conic(from, to, startDeg = -90, center)` | 锥形渐变（在起点处接缝，适合进度环） |
| `Paint::ConicLoop(a, b, startDeg = -90, center)` | 无接缝的锥形渐变（a → b → a） |
| `Paint::Spectrum(alpha = 1, from = 0, to = 1, angleDeg = 0)` | 沿方向的彩虹（两端渐隐，焦散、虹彩条纹） |

---

## 9. 主题

### 9.1 类型

**TextStyle**：`LargeTitle`、`Title1`、`Title2`、`Title3`、`Headline`、`Body`、`Callout`、`Subheadline`、`Footnote`、`Caption1`、`Caption2`、`Mono`。

**Spring**：`response`（无阻尼振荡周期，秒）、`damping`（阻尼比，1 = 临界）；预设 `Spring::Snappy()`、`Smooth()`、`Bouncy()`、`Gentle()`。

**GlassLook**：`Theme`、`Clear`、`Frosted`。

**GlassMaterial**（距离单位为缩放 1 时的 UI 单位）

| 字段 | 默认 | 说明 |
| --- | --- | --- |
| `blur` | 2 | 背景模糊（0 = 完全清透） |
| `refraction` | 8 | 边缘折射强度 |
| `bezel` | 40 | 弧形边缘半径（≥ 形状一半 = 整体是圆棒 / 圆顶） |
| `dispersion` | 0.35 | 色散（0..1） |
| `saturation` | 1.30 | 背景饱和度 |
| `brightness` | 0.03 | 提亮 |
| `specular` | 0.85 | 边缘高光 |
| `lightAngle` | -2.2 | 光照方向（弧度，默认左上） |
| `noise` | 0.012 | 颗粒，防色带 |
| `legibility` | 0 | 0..1 可读性（深色玻璃在亮背景上保持暗，反之亦然） |
| `magnify` | 0 | 放大镜（1 + magnify 倍） |
| `tint` | 白 0.18 | 着色（alpha = 着色量） |
| `rim` | 白 0.35 | 发丝线边框 |

**Palette**（`theme.colors`）：`accent`、`onAccent`；`label`、`secondaryLabel`、`tertiaryLabel`、`quaternaryLabel`；`background`、`secondaryBackground`、`tertiaryBackground`、`groupedBackground`、`secondaryGroupedBackground`、`tertiaryGroupedBackground`；`fill`、`secondaryFill`、`tertiaryFill`、`quaternaryFill`；`separator`、`opaqueSeparator`；系统色 `red`、`orange`、`yellow`、`green`、`mint`、`teal`、`cyan`、`blue`、`indigo`、`purple`、`pink`、`brown`、`gray`；表面 `windowSurface`、`cardSurface`、`controlKnob`、`shadow`、`highlight`。

**Materials**（`theme.materials`）：`window`（浮动窗口 / 面板）、`bar`（标签栏、Dock、工具栏）、`control`（玻璃按钮、旋钮、胶囊）、`popover`（菜单、提示）、`clear`（极少磨砂、强透镜）。

**Metrics**（`theme.metrics`，设计尺寸，使用时乘 `scale`）：`scale`、`windowRadius` 28、`cardRadius` 16、`controlRadius` 12、`cornerSmoothing` 0.6、`padding` 16、`spacing` 10、`rowHeight` 44、`controlHeight` 34、`iconTile` 28、`headerHeight` 54、`sectionSpacing` 22、`hairline` 1、`scrollIndicator` 5。

**Typography**（`theme.type`）：每个 `TextStyle` 的 `size[]` 与 `weight[]`。

**Motion**（`theme.motion`）：`fast`、`standard`、`bouncy`、`gentle` 四个弹簧，`themeTransition`（主题切换秒数，默认 0.45）。

**Theme**：`dark`、`colors`、`materials`、`metrics`、`type`、`motion`，辅助 `S(v)`。

### 9.2 函数

| 函数 | 说明 |
| --- | --- |
| `Theme ThemeLight()` / `Theme ThemeDark()` | 内置主题 |
| `Theme ThemeWithAccent(base, accent)` | 换强调色 |
| `Theme LerpTheme(a, b, t)` | 主题插值（过渡动画） |
| `ApplyThemeToImGuiStyle(theme, ImGuiStyle&)` | 把主题映射到 ImGui 样式，让原生控件融入 |

```cpp
Theme t = ui->GetTheme();                  // UI 线程
t.materials.window.blur = 20;
t.metrics.windowRadius = 32;
ui->SetTheme(t);                           // 任意线程，带过渡
```

---

## 10. 文字

WGT 使用自己的 DirectWrite 文字栈：按字体族名取系统字体（不加载 TTF 数据），逐字符系统回退（CJK、符号、任意已安装文字），OpenType 排版（字距、连字、双向、复杂文字、CJK 断行），光学尺寸，按物理像素密度的精确覆盖率光栅化，灰度或 ClearType 子像素抗锯齿。原生 ImGui 控件也通过字体加载器使用 DirectWrite 字形。

| 类型 / 函数 | 说明 |
| --- | --- |
| `FontWeight` | `Regular`、`Semibold`、`Bold`、`Mono` |
| `FontId` | 已注册字体句柄（0 = 无效） |
| `FontRef { FontId id; float size; }` | 字体 + 字号（UI 单位，已乘缩放） |
| `TextFlags_Ellipsis` / `TextFlags_AlignCenter` / `TextFlags_AlignRight` | 单行省略号 / 多行对齐 |
| `TextMetrics { Vec2 size; float baseline; int lines; }` | 测量结果 |
| `FontId RegisterFont(family, weight = 400, italic = false)` | 按族名注册（`AddFontFile` 注册的也可用；文件下一帧生效前先用界面字体）；未知族名回退到界面字体 |
| `FontId GetFontId(FontWeight)` | 内置字体 |
| `TextMetrics MeasureText(FontRef, text, end = null, wrapWidth = 0, flags = 0)` | 测量 |

```cpp
ui->AddFontFile(L"assets/fonts/MyBrand.ttf");
FontId brand = RegisterFont("My Brand", 700);
p.Text(pos, FontRef{brand, ui::S(28)}, Color::White(), "GAME OVER");
```

---

## 11. 动画

状态以 `ImGuiID` 为键保存在上下文中；每帧用**目标值**调用，得到当前的物理动画值（与帧率无关）。

| 函数 | 说明 |
| --- | --- |
| `anim::Float(id, target, const Spring* = null, initial = NAN)` | 标量弹簧动画（null = 主题标准弹簧；`initial` 首次使用时的起始值） |
| `anim::Float(const char* key, target, spring)` | 字符串键（在当前 ImGui ID 栈内哈希） |
| `anim::Vector(id, target, spring)` / `anim::Colour(id, target, spring)` | Vec2 / Color 动画 |
| `anim::Velocity(id)` | 当前速度（液态拉伸效果） |
| `anim::Set(id, value, velocity = 0)` | 直接跳到某值 |
| `anim::Kick(id, velocity)` | 施加冲量（点击“弹一下”） |
| `anim::Timer(id, duration, restart)` | 0..1 线性计时，`restart` 为 true 时重新开始 |
| `anim::Time()` / `anim::DeltaTime()` | 上下文创建以来的秒数 / 本帧间隔 |
| `SpringState { value, velocity; Step(target, spring, dt); Settled(target, eps) }` | 独立的弹簧积分器 |
| `ease::InOutCubic` `OutCubic` `OutBack` `OutExpo` `Smooth` | 缓动函数 |

```cpp
const float k = anim::Float(ImGui::GetID("hover"), hovered ? 1.0f : 0.0f, &theme.motion.fast);
```

---

## 12. 线程工具与回调

均为纯头文件，不跨 DLL 传引用，任何模块都能安全使用。

**`Property<T>`**：任意线程可读写的值，带版本号；对小的可平凡复制类型（bool、int、float、枚举、Vec2 …）无锁。

| 成员 | 说明 |
| --- | --- |
| `Get()` / `Set(v)` / `operator T` / `operator=` | 读写 |
| `Update([](T& v){ ... })` | 原子读-改-写 |
| `Version()` | 版本号 |
| `Changed(std::uint64_t& seen)` | 每次变化对每个观察者返回一次 true |

**`Channel<T>`**：多生产者 / 单消费者队列（日志、事件、命令）。`Push(v)`、`Emplace(args...)`、`Drain(fn)`（按推入顺序交给 fn，返回数量）、`Empty()`。

**`Latest<T>`**：无锁三缓冲，生产者 `Publish(v)`，UI 总是 `Fetch(out)` 到最新值（有新值时返回 true）。

**`Callback<R(Args...)>`**：跨 `wgt.dll` 边界唯一的可调用对象形式（三个裸指针，闭包在调用方模块里分配和释放，Debug 宿主可以直接用 Release 的 wgt.dll）。可由任意 lambda / 函数对象 / 函数指针构造，或 `Callback::FromRaw(fn, user, destroy)`；只能移动。`Task = Callback<void()>`，`PanelFn = Callback<void(Context&)>`，`LogFn = Callback<void(int, const char*)>`。

```cpp
Property<float> health{1.0f};
// 游戏线程
health.Set(0.42f);
// UI 线程
ui::ProgressBar(health.Get());
```

---

## 13. FramePacer

```cpp
FramePacer pacer;
pacer.SetTargetFps(120);   // 0 = 不限制
while (running) { /* 渲染 */ swapChain->Present(vsync ? 1 : 0, flags); pacer.Wait(); }
```

| 成员 | 说明 |
| --- | --- |
| `SetTargetFps(fps)` / `TargetFps()` | 目标帧率，≤ 0 = 不限制 |
| `Wait()` | 每帧在 Present 之后调用一次：高精度可等待计时器睡到快到期，再自旋最后不到 1 毫秒；截止时间按固定周期前进（不漂移），卡顿后重新对齐而不追帧 |
| `FrameMs()` / `AverageFps()` | 上一帧时长 / 约 0.5 秒平滑后的帧率 |

垂直同步由交换链的 Present 同步间隔负责，FramePacer 用于在其之下或没有它时限帧。

---

## 14. 自定义 HLSL 效果

```cpp
EffectId aurora = ui->RegisterEffect("aurora", kAuroraHlsl);   // 后台编译约 1 秒，之前用内置着色器
p.Rect(r, Style().Radius(18).Effect(aurora, 1.0f));            // p0..p3 进入 fx.params
```

源码必须定义 `float4 WgtEffect(WgtFx fx)`，返回**预乘 alpha** 颜色。`WgtFx` 字段：

| 字段 | 说明 |
| --- | --- |
| `float2 pos` | ImGui 空间位置 |
| `float2 uv` | 形状边界内 0..1 |
| `float2 size` | 形状边界尺寸 |
| `float2 screenUV` | 渲染目标内 0..1 |
| `float sd` | 到形状的有符号距离（内部为负） |
| `float coverage` | 抗锯齿覆盖率 |
| `float px` | 每像素的 ImGui 单位 |
| `float time` | 秒 |
| `float4 params` | `Style::Effect` 的 p0..p3 |
| `float4 fill` | 计算好的填充色（直通 alpha） |

可用函数：`float3 WgtBackdrop(float2 screenUV, float blurPx)`（采样模糊后的背景）、`float4 WgtTexture(float2 uv)`（`Style::Image` 的贴图）。

---

## 15. 插件

```cpp
class MyPlugin : public wgt::IPlugin
{
public:
    const char* Name() const override { return "My Plugin"; }
    void OnAttach(wgt::Context& ctx) override { /* 注册面板、效果 */ }
    void OnFrame(wgt::Context& ctx) override { /* 每帧 UI */ }
    void OnDetach(wgt::Context& ctx) override {}
};
WGT_DECLARE_PLUGIN(MyPlugin)   // 导出 WgtCreatePlugin / WgtDestroyPlugin，并检查 ABI 版本
```

`IPlugin` 的三个回调都在 UI 线程执行。加载：`Context::LoadPlugin(L"plugin.dll")`、`LoadPluginsFromDirectory(L"plugins")`；进程内插件：`AddPlugin(&plugin)`（不接管所有权）。ABI 版本不一致的插件会被拒绝加载。

---

## 16. 自定义渲染后端

DX11 / DX12 后端随 `wgt.dll` 提供。移植到其他 API（Vulkan、引擎自带渲染器）时实现 `IRenderBackend`，通过 `ContextDesc::customBackend` 传入。后端需要：

1. 遵循 Dear ImGui 的贴图协议（`ImDrawData::Textures`、`ImGuiBackendFlags_RendererHasTextures`）；
2. 渲染普通 `ImDrawCmd` 几何；
3. 解释 `fx::CommandCallback()` 回调里的 `fx::Command`（形状、发光层、边缘渐隐）。忽略 FX 命令的后端仍能渲染全部文字和 ImGui 几何。

| 方法 | 线程 | 说明 |
| --- | --- | --- |
| `const char* Name()` | — | 名称 |
| `bool Init(ImGuiIO&)` | UI | ImGui 上下文创建后调用一次（在这里设置 `io.BackendFlags`） |
| `void Shutdown()` | UI | 释放全部 GPU 对象（包括 ImGui 拥有的贴图） |
| `void RenderDrawData(ImDrawData*, const RenderTarget&, const BackendFrameInfo&)` | UI | 渲染一帧 |
| `ImTextureID CreateTexture(rgba8, w, h)` / `DestroyTexture(tex)` | 任意 | 图片贴图 |
| `SetEffectSource(id, name, hlsl)` | 任意 | 自定义效果源码（可选） |
| `bool QueryTargetSize(target, w, h)` | 任意 | 渲染目标尺寸（自动渲染缩放用） |
| `InvalidateDeviceObjects()` / `CollectStats(FrameStats&)` / `SetLogCallback(...)` | — | 可选 |

**BackendFrameInfo**：`time`、`deltaTime`、`maxBackdropCaptures`、文字合成参数 `textGamma`、`textGrayscaleContrast`、`textClearTypeContrast`、`textClearTypeLevel`。FX 命令与实例的二进制布局见 `wgt/fx.hpp`（`fx::Instance` 必须与着色器中的 `FxInst` 一致）。

---

## 17. 图标

图标来自 Windows 系统图标字体（Windows 11 为 Segoe Fluent Icons，Windows 10 为 Segoe MDL2 Assets），不附带资源、没有授权问题。这些字体的任意码位都能用；`wgt::icons` 里是精选常量：

`Menu` `Wifi` `Bluetooth` `Connect` `Vpn` `Brightness` `MapPin` `Moon` `Airplane` `ChevronDown` `ChevronUp` `Edit` `Add` `Close` `More` `Settings` `Video` `Mail` `People` `Phone` `Pin` `Shop` `Stop` `Link` `Filter` `Apps` `ZoomIn` `ZoomOut` `Microphone` `Search` `Camera` `Attach` `Send` `Forward` `Back` `Refresh` `Share` `Lock` `Star` `StarFill` `Remove` `Checkmark` `FullScreen` `Delete` `Save` `Mute` `Cloud` `ChevronLeft` `ChevronRight` `Volume` `Play` `Pause` `Brush` `Globe` `Contact` `Error` `Unlock` `Calendar` `Palette` `Warning` `Flag` `Power` `Game` `Home` `History` `Location` `Recent` `View` `Sync` `Download` `Help` `Upload` `Document` `Folder` `Copy` `Music` `World` `Photo` `Code` `Lightning` `Info` `Mouse` `Diagnostic` `Equalizer` `Shield` `Lightbulb` `Bell` `Heart` `HeartFill` `Bug` `Hide`

```cpp
ui::Button("设置", {.icon = icons::Settings});
p.Icon(center, icons::Wifi, ui::S(18), Color::White());
p.Icon(center, 0xE8BE, ui::S(18), Color::White());   // 任意系统图标字体码位
```

---

## 18. 版本与 ABI

| 宏 / 函数 | 值 |
| --- | --- |
| `WGT_VERSION_MAJOR` / `MINOR` / `PATCH` / `WGT_VERSION_STRING` | 1 / 1 / 0 / `"1.1.0"` |
| `WGT_ABI_VERSION` | 8（导出的结构体或接口布局变化时递增，插件加载时检查） |
| `GetVersionString()` / `GetAbiVersion()` | 运行时查询 DLL 的版本 |

公共 API 导出 C++ 类，只支持 MSVC x64（VS 2022 v143 已验证）。本版的默认行为与接口变化见 [INTEGRATION.md](INTEGRATION.md) 第 13.1 节。
