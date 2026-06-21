# QtQuickComponents

通用 Qt Quick UI 组件库，完全独立于业务项目，通过 `FetchContent` 引入。

## 项目结构

```text
src/
  theme/      # ComponentTheme 主题单例与设计 token
  controls/   # Button、TextField、Label 等通用 QML 控件
  timeline/   # TimelineView 及其 C++ 模型、视口、刻度和轨道渲染辅助类型
themes/       # JSON 主题文件，默认包含 dark / light
tests/
  qml/        # QML QuickTest 用例
```

公开组件导出到 QML 模块：`QuickUI.Components 1.0`。库内部实现组件导出到 `QuickUI.Components.impl 1.0`，应用通常只需要导入公开模块；构建和部署时需要确保 impl 模块随主模块一起可被 QML 引擎找到。

基础控件基于 `QtQuick.Controls.Basic` 扩展，保留 Qt Quick Controls 的焦点、键盘、hover/pressed 等状态语义，同时使用 `ComponentTheme` 定制视觉样式。

## 组件列表

| 组件 | 说明 |
|---|---|
| `IconButton` | 纯文本/Emoji 图标按钮，支持 hover 效果和 tooltip |
| `ComboBox` | 通用下拉选择框，支持 textRole/valueRole、键盘焦点和主题化弹出菜单 |
| `ProgressSlider` | 自定义外观滑块，支持点击任意位置跳转、bufferPosition |
| `Button` | 通用按钮，filled / outline / ghost 三种变体，支持 loading 状态 |
| `Label` | 语义化文本标签，body / label / caption / heading 四种角色 |
| `TextField` | 单行输入框，支持标签、占位、错误提示、密码模式、清除按钮 |
| `TimelineView` | 时间轴视图，包含录像片段轨道、刻度尺、播放头、缩放/拖拽/seek 交互 |
| `SingleWnd` | 单窗口视频组件，包含视频面、无信号层、OSD、边框告警、标题栏和拖拽交换 |
| `ComponentTheme` | C++ 主题单例，提供颜色/字体/尺寸/动画 token，支持 Dark/Light/Custom |
| `TimelineModel` | C++ 时间轴片段模型，向 QML 暴露录像片段和总时间范围 |
| `WndViewModel` | C++ 单窗口视图模型，向 `SingleWnd` 提供窗口状态、通道、录像、OSD 和事件信号 |

## 引入方式（FetchContent）

在项目根 `CMakeLists.txt` 中：

```cmake
include(FetchContent)
FetchContent_Declare(
    QtQuickComponents
    GIT_REPOSITORY https://github.com/Zacus/QtQuickComponents.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(QtQuickComponents)
```

链接到你的 target。完整组件集包含内部 QML 模块，推荐同时链接 `QtQuickComponentsImpl`，这样示例、测试和安装布局都能生成主模块与 impl 模块：

```cmake
target_link_libraries(YourApp PRIVATE
    QtQuickComponents
    QtQuickComponentsImpl
)

target_compile_definitions(YourApp PRIVATE
    QML_IMPORT_PATH="${CMAKE_BINARY_DIR}"
)
```

在 `main.cpp` 中添加搜索路径，静态构建时还需导入插件：

```cpp
engine.addImportPath(QStringLiteral(QML_IMPORT_PATH));

#if defined(QT_STATIC)
#  include <QtPlugin>
   Q_IMPORT_QML_PLUGIN(QtQuickComponentsPlugin)
   Q_IMPORT_QML_PLUGIN(QtQuickComponentsImplPlugin)
#endif
```

## QML 中使用

```qml
import QuickUI.Components 1.0

// ── 主题 ─────────────────────────────────────────────────────
Component.onCompleted: {
    ComponentTheme.style = ComponentTheme.Light

    // JSON 主题：从 themes/<id>.json 加载，文件修改后可热加载
    ComponentTheme.themeDirectory = "/path/to/themes"
    ComponentTheme.hotReloadEnabled = true
    ComponentTheme.loadTheme("dark")

    // Custom 模式：逐项覆盖
    ComponentTheme.setAccent("#ff6b6b")
    ComponentTheme.setFontFamily("Inter")
    ComponentTheme.setButtonRadius(ComponentTheme.Round)
    ComponentTheme.reducedMotion = true   // 无障碍低动效
}

// ── Button ────────────────────────────────────────────────────
Button { text: "确认"; onClicked: doSomething() }
Button { text: "取消"; variant: "ghost" }
Button { text: "提交"; loading: true }
Button { text: "删除"; variant: "outline"; enabled: false }

// ── Label ─────────────────────────────────────────────────────
Label { role: "heading"; text: "账号设置" }
Label { text: "请填写以下信息" }
Label { role: "label";   text: "邮箱地址" }
Label { role: "caption"; text: "最后修改于 3 分钟前" }

// ── TextField ─────────────────────────────────────────────────
TextField {
    label:       "邮箱"
    placeholder: "you@example.com"
    errorText:   root.emailError
    clearable:   true
    onAccepted:  root.submit()
}
TextField {
    label:    "密码"
    password: true
}

// ── IconButton ────────────────────────────────────────────────
IconButton {
    iconText: "▶"
    tooltip:  "Play"
    onClicked: player.play()
}

// ── ComboBox ─────────────────────────────────────────────────
ComboBox {
    model: ListModel {
        ListElement { code: "en_US"; label: "EN" }
        ListElement { code: "zh_CN"; label: "中文" }
    }
    textRole: "label"
    valueRole: "code"
    popupMaxVisibleItems: 4
    onActivated: function(index) {
        console.log(model.get(index).code)
    }
}

// ── ProgressSlider ────────────────────────────────────────────
ProgressSlider {
    from:           0.0
    to:             1.0
    value:          player.progress
    bufferPosition: player.bufferedPosition
    onMoved:        player.seek(value)
}

// ── TimelineView ─────────────────────────────────────────────
TimelineModel {
    id: timelineModel
    Component.onCompleted: {
        addSegment(0, 900000, TimelineEnums.SegmentNormal)
        addSegment(1200000, 2300000, TimelineEnums.SegmentMotion)
        addSegment(2600000, 3500000, TimelineEnums.SegmentAlarm)
    }
}

TimelineView {
    width: 640
    height: 120
    model: timelineModel
    currentTime: player.positionMs
    followMode: TimelineEnums.FollowEdge
    onSeeked: function(timeMs) {
        player.seek(timeMs)
    }
    onPlayRequested: function(timeMs) {
        player.seek(timeMs)
        player.play()
    }
}
```

`TimelineView` 的底色、刻度尺底色和播放头默认跟随主题：`trackColor` / `rulerBg` 使用 `ComponentTheme.trackBg`，`playheadColor` 使用 `ComponentTheme.textPrimary`。录像片段颜色通过 `segmentColors` 表达业务语义，默认按普通/移动/告警区分，也可以按项目需要覆盖。

## JSON 主题

`ComponentTheme` 支持从 JSON 文件加载完整主题 token。默认主题文件位于仓库 `themes/` 目录，构建时会复制到 `${QTC_QML_OUTPUT_BASE}/themes`，安装时会复制到 QtQuickComponents 的 share 目录。

节选示例：

```json
{
  "id": "dark",
  "name": "Dark",
  "colors": {
    "accent": "#7c6fff",
    "textPrimary": "#f0f0f5",
    "surface": "#1e1e2a"
  },
  "sizes": {
    "buttonSize": 34,
    "buttonRadius": 6
  },
  "fonts": {
    "fontFamily": "",
    "fontSize": 16
  },
  "motion": {
    "durationFast": 80,
    "durationNormal": 120,
    "reducedMotion": false
  }
}
```

实际主题文件必须包含所有 `ComponentTheme` token，完整格式可参考 `themes/dark.json` 和 `themes/light.json`。加载时会先完整校验 JSON，只有全部字段合法才会应用；坏 JSON 或非法颜色不会污染当前主题。启用 `hotReloadEnabled` 后，当前主题文件变化会通过 `QFileSystemWatcher` 自动重新加载并触发 `styleChanged()`。

## 字体 Token

```qml
// 字号
font.pixelSize: ComponentTheme.fontSize        // 正文 16px
font.pixelSize: ComponentTheme.fontSizeLabel   // 标签 13px
font.pixelSize: ComponentTheme.fontSizeCaption // 说明 11px

// 字族（空串 = 系统默认，可通过 setFontFamily() 覆盖）
font.family: ComponentTheme.fontFamily

// 字重（对应 QFont::Weight 整数，可直接赋给 font.weight）
font.weight: ComponentTheme.fontWeightNormal   // 400
font.weight: ComponentTheme.fontWeightMedium   // 500
font.weight: ComponentTheme.fontWeightBold     // 700
```

## 要求

- Qt 6.4+
- CMake 3.21+
- C++17

## Demo Apps

仓库包含两个 demo app：

- `qtc_demo`：验证 `import QuickUI.Components 1.0`、基础控件视觉状态、主题切换和 `TimelineView` 基础交互。
- `qtc_single_wnd_demo`：验证 `SingleWnd` 的视频面、状态层、OSD、标题栏、告警边框、拖拽交换和最大化交互。

```sh
cmake -S . -B build -DQTC_BUILD_EXAMPLES=ON
cmake --build build --target qtc_demo
./build/examples/demo/qtc_demo
```

运行 SingleWnd 示例：

```sh
cmake --build build --target qtc_single_wnd_demo
./build/examples/single_wnd_demo/qtc_single_wnd_demo
```

Headless import smoke test:

```sh
QT_QPA_PLATFORM=offscreen ./build/examples/demo/qtc_demo --quit-after-ms 100
QT_QPA_PLATFORM=offscreen ./build/examples/single_wnd_demo/qtc_single_wnd_demo --quit-after-ms 100
```

## 版本规则

库版本遵循 SemVer，并以仓库根目录 `VERSION.txt` 文件作为唯一来源。CMake package 使用完整 `MAJOR.MINOR.PATCH`，QML 模块版本使用 `MAJOR.MINOR`。

- `MAJOR`：破坏公开 C++ API、QML 类型、属性语义或 import 兼容性。
- `MINOR`：向后兼容的新组件、新属性或新能力。
- `PATCH`：bugfix、性能优化、文档修正或不改变公开 API 的内部重构。

发布 tag 使用 `vMAJOR.MINOR.PATCH`，例如 `v1.0.0`。
