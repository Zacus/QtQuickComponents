# QtQuickComponents

通用 Qt Quick UI 组件库，完全独立于业务项目，通过 `FetchContent` 引入。

## 组件列表

| 组件 | 说明 |
|---|---|
| `IconButton` | 纯文本/Emoji 图标按钮，支持 hover 效果和 tooltip |
| `ProgressSlider` | 自定义外观滑块，支持点击任意位置跳转 |
| `ComponentTheme` | C++ 主题单例，提供颜色/尺寸 token，支持 Dark/Light/Custom |

## 引入方式（FetchContent）

在项目根 `CMakeLists.txt` 中：

```cmake
include(FetchContent)
FetchContent_Declare(
    QtQuickComponents
    GIT_REPOSITORY https://github.com/yourname/QtQuickComponents.git
    GIT_TAG        v1.0.0
)
FetchContent_MakeAvailable(QtQuickComponents)
```

链接到你的 target：

```cmake
target_link_libraries(YourApp PRIVATE QtQuickComponents)

# 让 QML 引擎能找到模块的 qmldir
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

// 切换主题（可选，默认 Dark）
Component.onCompleted: {
    ComponentTheme.style = ComponentTheme.Light
    // 或自定义强调色
    ComponentTheme.setAccent("#ff6b6b")
}

IconButton {
    iconText: "▶"
    tooltip:  "Play"
    onClicked: player.play()
}

ProgressSlider {
    from:  0.0
    to:    1.0
    value: player.progress
    onMoved: player.seek(value)
}
```

## 要求

- Qt 6.4+
- CMake 3.21+
- C++17
