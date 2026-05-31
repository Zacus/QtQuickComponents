# QtQuickComponents

通用 Qt Quick UI 组件库，完全独立于业务项目，通过 `FetchContent` 引入。

## 项目结构

```text
src/
  theme/      # ComponentTheme 主题单例与设计 token
  controls/   # Button、TextField、Label 等通用 QML 控件
  timeline/   # TimelineView 及其 C++ 模型、视口、刻度和轨道渲染辅助类型
tests/
  qml/        # QML QuickTest 用例
```

所有组件仍导出到同一个 QML 模块：`QuickUI.Components 1.0`。

基础控件基于 `QtQuick.Controls.Basic` 扩展，保留 Qt Quick Controls 的焦点、键盘、hover/pressed 等状态语义，同时使用 `ComponentTheme` 定制视觉样式。

## 组件列表

| 组件 | 说明 |
|---|---|
| `IconButton` | 纯文本/Emoji 图标按钮，支持 hover 效果和 tooltip |
| `ProgressSlider` | 自定义外观滑块，支持点击任意位置跳转、bufferPosition |
| `Button` | 通用按钮，filled / outline / ghost 三种变体，支持 loading 状态 |
| `Label` | 语义化文本标签，body / label / caption / heading 四种角色 |
| `TextField` | 单行输入框，支持标签、占位、错误提示、密码模式、清除按钮 |
| `ComponentTheme` | C++ 主题单例，提供颜色/字体/尺寸/动画 token，支持 Dark/Light/Custom |

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

链接到你的 target：

```cmake
target_link_libraries(YourApp PRIVATE QtQuickComponents)

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
#endif
```

## QML 中使用

```qml
import QuickUI.Components 1.0

// ── 主题 ─────────────────────────────────────────────────────
Component.onCompleted: {
    ComponentTheme.style = ComponentTheme.Light

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

// ── ProgressSlider ────────────────────────────────────────────
ProgressSlider {
    from:           0.0
    to:             1.0
    value:          player.progress
    bufferPosition: player.bufferedPosition
    onMoved:        player.seek(value)
}
```

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

## Demo App

仓库包含一个最小 demo app，用于人工验证 `import QuickUI.Components 1.0`、控件视觉状态和 `TimelineView` 基础交互。

```sh
cmake -S . -B build -DQTC_BUILD_EXAMPLES=ON
cmake --build build --target qtc_demo
./build/examples/demo/qtc_demo
```

Headless import smoke test:

```sh
./build/examples/demo/qtc_demo -platform offscreen --quit-after-ms 100
```

## 版本规则

库版本遵循 SemVer，并以仓库根目录 `VERSION.txt` 文件作为唯一来源。CMake package 使用完整 `MAJOR.MINOR.PATCH`，QML 模块版本使用 `MAJOR.MINOR`。

- `MAJOR`：破坏公开 C++ API、QML 类型、属性语义或 import 兼容性。
- `MINOR`：向后兼容的新组件、新属性或新能力。
- `PATCH`：bugfix、性能优化、文档修正或不改变公开 API 的内部重构。

发布 tag 使用 `vMAJOR.MINOR.PATCH`，例如 `v1.0.0`。
