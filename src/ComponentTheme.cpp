#include "ComponentTheme.h"

ComponentTheme::ComponentTheme(QObject* parent)
    : QObject(parent)
{
    applyDark();
}

void ComponentTheme::setStyle(Style s)
{
    if (m_style == s) return;
    m_style = s;
    switch (s) {
    case Dark:   applyDark();  break;
    case Light:  applyLight(); break;
    case Custom: break;   // 保留当前值，由外部逐项覆盖
    }
    emit styleChanged();
}

void ComponentTheme::applyDark()
{
    // ── 强调色 ────────────────────────────────────────────────
    m_accent         = QColor("#7c6fff");
    m_accentHover    = QColor("#9d90ff");
    m_accentPressed  = QColor("#9d90ff");
    m_accentDisabled = QColor("#44445a");

    // ── 图标色 ────────────────────────────────────────────────
    m_iconColor        = QColor("#ffffff99");
    m_iconColorPressed = QColor("#ffffffcc");

    // ── 按钮背景 ──────────────────────────────────────────────
    m_buttonHover   = QColor("#ffffff0e");
    m_buttonPressed = QColor("#ffffff18");

    // ── 滑轨 ──────────────────────────────────────────────────
    m_trackBg      = QColor("#2a2a3a");
    m_trackBuffer  = QColor("#ffffff12");
    m_handleBorder = QColor("#ffffff30");

    // ── 文字色 ────────────────────────────────────────────────
    m_textPrimary   = QColor("#f0f0f5");
    m_textSecondary = QColor("#9090a8");
    m_textDisabled  = QColor("#50505f");
    m_textOnAccent  = QColor("#ffffff");

    // ── 表面色 ────────────────────────────────────────────────
    m_surface      = QColor("#1e1e2a");
    m_surfaceHover = QColor("#26263a");
    m_separator    = QColor("#ffffff14");

    // ── 输入框色 ──────────────────────────────────────────────
    m_inputBg          = QColor("#14141e");
    m_inputBorder      = QColor("#ffffff20");
    m_inputFocus       = QColor("#7c6fff");
    m_inputText        = QColor("#f0f0f5");
    m_inputPlaceholder = QColor("#50505f");

    applyDefaultSizes();
}

void ComponentTheme::applyLight()
{
    // ── 强调色 ────────────────────────────────────────────────
    m_accent         = QColor("#5b4de8");
    m_accentHover    = QColor("#7b6fff");
    m_accentPressed  = QColor("#4a3dd4");
    m_accentDisabled = QColor("#b0acd8");

    // ── 图标色 ────────────────────────────────────────────────
    m_iconColor        = QColor("#00000099");
    m_iconColorPressed = QColor("#000000cc");

    // ── 按钮背景 ──────────────────────────────────────────────
    m_buttonHover   = QColor("#0000000e");
    m_buttonPressed = QColor("#00000018");

    // ── 滑轨 ──────────────────────────────────────────────────
    m_trackBg      = QColor("#d0d0e0");
    m_trackBuffer  = QColor("#00000012");
    m_handleBorder = QColor("#00000030");

    // ── 文字色 ────────────────────────────────────────────────
    m_textPrimary   = QColor("#18181f");
    m_textSecondary = QColor("#60607a");
    m_textDisabled  = QColor("#a8a8be");
    m_textOnAccent  = QColor("#ffffff");

    // ── 表面色 ────────────────────────────────────────────────
    m_surface      = QColor("#f4f4f8");
    m_surfaceHover = QColor("#ebebf2");
    m_separator    = QColor("#00000014");

    // ── 输入框色 ──────────────────────────────────────────────
    m_inputBg          = QColor("#ffffff");
    m_inputBorder      = QColor("#00000020");
    m_inputFocus       = QColor("#5b4de8");
    m_inputText        = QColor("#18181f");
    m_inputPlaceholder = QColor("#a8a8be");

    applyDefaultSizes();
}

void ComponentTheme::setAccent(const QColor& c)
{
    m_style          = Custom;
    m_accent         = c;
    m_accentHover    = c.lighter(115);
    m_accentPressed  = c.lighter(115);
    m_accentDisabled = c.darker(160);
    // 输入框焦点色跟随强调色
    m_inputFocus = c;
    emit styleChanged();
}

void ComponentTheme::setButtonRadius(int r)
{
    m_style        = Custom;
    m_buttonRadius = r;
    // 输入框圆角与按钮保持一致
    m_inputRadius  = r;
    emit styleChanged();
}

void ComponentTheme::setFontFamily(const QString& family)
{
    m_style      = Custom;
    m_fontFamily = family;
    emit styleChanged();
}

void ComponentTheme::applyDefaultSizes()
{
    // Dark 和 Light 共用同一套尺寸/字体默认值，在此集中维护。
    // Custom 模式下可通过各 set*() 方法逐项覆盖。
    m_buttonSize   = 34;
    m_buttonRadius = 6;
    m_inputHeight  = 36;
    m_inputRadius  = 6;
    m_trackHeight  = 4;
    m_handleSize   = 14;

    // 字体：空串表示使用平台默认字体，Qt 会自动选择最合适的系统 UI 字体
    m_fontFamily      = QString();
    m_fontSize        = 16;
    m_fontSizeLabel   = 13;
    m_fontSizeCaption = 11;
}

void ComponentTheme::setReducedMotion(bool reduced)
{
    if (m_reducedMotion == reduced) return;
    m_reducedMotion = reduced;
    emit styleChanged();
}
