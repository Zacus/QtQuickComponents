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
    m_trackBg     = QColor("#2a2a3a");
    m_trackBuffer = QColor("#ffffff12");
    m_handleBorder = QColor("#ffffff30");

    // ── 尺寸（交由 applyDefaultSizes 统一赋值）───────────────
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

    // ── 尺寸（交由 applyDefaultSizes 统一赋值）───────────────
    applyDefaultSizes();
}

void ComponentTheme::setAccent(const QColor& c)
{
    m_style          = Custom;
    m_accent         = c;
    // 自动派生悬停色（稍亮）和按下色（同悬停）
    m_accentHover    = c.lighter(115);
    m_accentPressed  = c.lighter(115);
    m_accentDisabled = c.darker(160);
    emit styleChanged();
}

void ComponentTheme::setButtonRadius(int r)
{
    m_style        = Custom;
    m_buttonRadius = r;
    emit styleChanged();
}

void ComponentTheme::applyDefaultSizes()
{
    // Dark 和 Light 共用同一套尺寸默认值，在此集中维护。
    // Custom 模式下可通过 setButtonRadius() 等方法逐项覆盖。
    m_buttonSize   = 34;
    m_buttonRadius = 6;
    m_fontSize     = 16;
    m_trackHeight  = 4;
    m_handleSize   = 14;
}

void ComponentTheme::setReducedMotion(bool reduced)
{
    if (m_reducedMotion == reduced) return;
    m_reducedMotion = reduced;
    // durationFast / durationNormal 的返回值因此改变，通知 QML 侧重新读取
    emit styleChanged();
}
