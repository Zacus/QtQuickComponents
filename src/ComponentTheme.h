#pragma once

#include <QObject>
#include <QColor>
#include <QQmlEngine>

/**
 * @brief 组件库全局主题单例
 *
 * 在 QML 中作为附加属性使用：
 *   import QuickUI.Components 1.0
 *   color: ComponentTheme.accent
 *
 * 支持运行时切换主题（Dark / Light / Custom）：
 *   ComponentTheme.style = ComponentTheme.Light
 */
class ComponentTheme : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    // ── 主题风格枚举 ──────────────────────────────────────────
    enum Style { Dark, Light, Custom };
    Q_ENUM(Style)

    // ── 圆角枚举 ─────────────────────────────────────────────
    enum Radius { Sharp = 0, Soft = 4, Round = 8, Pill = 999 };
    Q_ENUM(Radius)

    static ComponentTheme* create(QQmlEngine*, QJSEngine*)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    static ComponentTheme& instance()
    {
        static ComponentTheme s;
        return s;
    }

    // ── 当前风格 ──────────────────────────────────────────────
    Q_PROPERTY(Style style READ style WRITE setStyle NOTIFY styleChanged)
    Style style() const { return m_style; }
    void  setStyle(Style s);

    // ── 核心色彩 Token ────────────────────────────────────────
    Q_PROPERTY(QColor accent          READ accent          NOTIFY styleChanged)
    Q_PROPERTY(QColor accentHover     READ accentHover     NOTIFY styleChanged)
    Q_PROPERTY(QColor accentPressed   READ accentPressed   NOTIFY styleChanged)
    Q_PROPERTY(QColor accentDisabled  READ accentDisabled  NOTIFY styleChanged)

    Q_PROPERTY(QColor iconColor        READ iconColor        NOTIFY styleChanged)
    Q_PROPERTY(QColor iconColorPressed READ iconColorPressed NOTIFY styleChanged)

    Q_PROPERTY(QColor buttonHover   READ buttonHover   NOTIFY styleChanged)
    Q_PROPERTY(QColor buttonPressed READ buttonPressed NOTIFY styleChanged)

    Q_PROPERTY(QColor trackBg      READ trackBg      NOTIFY styleChanged)
    Q_PROPERTY(QColor trackBuffer  READ trackBuffer  NOTIFY styleChanged)
    Q_PROPERTY(QColor handleBorder READ handleBorder NOTIFY styleChanged)

    // ── 尺寸 Token ────────────────────────────────────────────
    Q_PROPERTY(int buttonSize   READ buttonSize   NOTIFY styleChanged)
    Q_PROPERTY(int buttonRadius READ buttonRadius NOTIFY styleChanged)
    Q_PROPERTY(int fontSize     READ fontSize     NOTIFY styleChanged)
    Q_PROPERTY(int trackHeight  READ trackHeight  NOTIFY styleChanged)
    Q_PROPERTY(int handleSize   READ handleSize   NOTIFY styleChanged)

    // ── 动画时长 Token ────────────────────────────────────────
    Q_PROPERTY(int durationFast   READ durationFast   CONSTANT)
    Q_PROPERTY(int durationNormal READ durationNormal CONSTANT)

    // ── 颜色访问器 ────────────────────────────────────────────
    QColor accent()          const { return m_accent; }
    QColor accentHover()     const { return m_accentHover; }
    QColor accentPressed()   const { return m_accentPressed; }
    QColor accentDisabled()  const { return m_accentDisabled; }
    QColor iconColor()        const { return m_iconColor; }
    QColor iconColorPressed() const { return m_iconColorPressed; }
    QColor buttonHover()     const { return m_buttonHover; }
    QColor buttonPressed()   const { return m_buttonPressed; }
    QColor trackBg()         const { return m_trackBg; }
    QColor trackBuffer()     const { return m_trackBuffer; }
    QColor handleBorder()    const { return m_handleBorder; }

    // ── 尺寸访问器 ────────────────────────────────────────────
    int buttonSize()   const { return m_buttonSize; }
    int buttonRadius() const { return m_buttonRadius; }
    int fontSize()     const { return m_fontSize; }
    int trackHeight()  const { return m_trackHeight; }
    int handleSize()   const { return m_handleSize; }

    // ── 动画时长 ──────────────────────────────────────────────
    int durationFast()   const { return 80; }
    int durationNormal() const { return 120; }

    // ── Custom 风格下允许逐项覆盖 ─────────────────────────────
    Q_INVOKABLE void setAccent(const QColor& c);
    Q_INVOKABLE void setButtonRadius(int r);

signals:
    void styleChanged();

private:
    explicit ComponentTheme(QObject* parent = nullptr);
    void applyDark();
    void applyLight();

    Style  m_style = Dark;

    QColor m_accent;
    QColor m_accentHover;
    QColor m_accentPressed;
    QColor m_accentDisabled;
    QColor m_iconColor;
    QColor m_iconColorPressed;
    QColor m_buttonHover;
    QColor m_buttonPressed;
    QColor m_trackBg;
    QColor m_trackBuffer;
    QColor m_handleBorder;

    int m_buttonSize   = 34;
    int m_buttonRadius = 6;
    int m_fontSize     = 16;
    int m_trackHeight  = 4;
    int m_handleSize   = 14;
};
