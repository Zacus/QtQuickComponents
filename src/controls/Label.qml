import QtQuick
import QuickUI.Components 1.0

// 语义化文本标签，根据 role 自动应用字号、字重、颜色。
//
// role 取值：
//   "body"    — 正文，fontSize / Normal / textPrimary（默认）
//   "label"   — 标签/辅助，fontSizeLabel / Normal / textSecondary
//   "caption" — 说明，fontSizeCaption / Normal / textDisabled
//   "heading" — 标题，fontSize / Bold / textPrimary
//
// 所有属性可单独覆盖，role 只是提供合理默认值的快捷方式：
//   Label { role: "label"; color: ComponentTheme.accent }
//
// 用法示例：
//   Label { text: "用户名" }
//   Label { role: "heading"; text: "设置" }
//   Label { role: "caption"; text: "最后修改于 3 分钟前" }

Text {
    id: root

    property string role: "body"   // "body" | "label" | "caption" | "heading"

    font.family:    ComponentTheme.fontFamily
    font.pixelSize: _resolveSize()
    font.weight:    _resolveWeight()
    color:          _resolveColor()

    wrapMode: Text.WordWrap

    // ── 角色解析 ─────────────────────────────────────────────
    function _resolveSize() {
        switch (root.role) {
        case "caption": return ComponentTheme.fontSizeCaption;
        case "label":   return ComponentTheme.fontSizeLabel;
        default:        return ComponentTheme.fontSize;   // "body" + "heading"
        }
    }

    function _resolveWeight() {
        return root.role === "heading"
               ? ComponentTheme.fontWeightBold
               : ComponentTheme.fontWeightNormal;
    }

    function _resolveColor() {
        switch (root.role) {
        case "caption": return ComponentTheme.textDisabled;
        case "label":   return ComponentTheme.textSecondary;
        default:        return ComponentTheme.textPrimary;   // "body" + "heading"
        }
    }
}
