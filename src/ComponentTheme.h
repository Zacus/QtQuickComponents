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
 *
 * @note 线程安全约束：本单例基于函数级静态变量（C++11 保证初始化线程安全），
 *       但属性读写及信号发射均未加锁，须在同一线程（通常为 GUI 线程）访问。
 *       多 QQmlEngine 场景（如 Qt for Android 多线程渲染）下共享此单例时，
 *       调用方有责任确保所有操作在同一线程执行，或在外部加锁。
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
    // 使用 NOTIFY styleChanged 而非 CONSTANT，以便未来支持
    // 无障碍"低动效"模式：reducedMotion = true 时两值均归零。
    Q_PROPERTY(int  durationFast    READ durationFast    NOTIFY styleChanged)
    Q_PROPERTY(int  durationNormal  READ durationNormal  NOTIFY styleChanged)
    Q_PROPERTY(bool reducedMotion   READ reducedMotion   WRITE setReducedMotion NOTIFY styleChanged)

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
    int  durationFast()   const { return m_reducedMotion ? 0 : m_durationFast; }
    int  durationNormal() const { return m_reducedMotion ? 0 : m_durationNormal; }
    bool reducedMotion()  const { return m_reducedMotion; }
    void setReducedMotion(bool reduced);

    // ── Custom 风格下允许逐项覆盖 ─────────────────────────────
    Q_INVOKABLE void setAccent(const QColor& c);
    Q_INVOKABLE void setButtonRadius(int r);

signals:
    void styleChanged();

private:
    explicit ComponentTheme(QObject* parent = nullptr);
    void applyDark();
    void applyLight();
    void applyDefaultSizes();   // Dark/Light 共用尺寸，避免重复

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

    // 动画时长基准值（reducedMotion = true 时对外返回 0）
    int  m_durationFast   = 80;
    int  m_durationNormal = 120;
    bool m_reducedMotion  = false;
};
