#pragma once

#include <QObject>
#include <QColor>
#include <QString>
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
 * 字体 token 用法：
 *   font.family:     ComponentTheme.fontFamily
 *   font.pixelSize:  ComponentTheme.fontSize        // 正文 16px
 *   font.pixelSize:  ComponentTheme.fontSizeLabel   // 标签 13px
 *   font.pixelSize:  ComponentTheme.fontSizeCaption // 说明 11px
 *   font.weight:     ComponentTheme.fontWeightNormal
 *   font.weight:     ComponentTheme.fontWeightMedium
 *   font.weight:     ComponentTheme.fontWeightBold
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

    // ── 字重枚举（与 QFont::Weight 对齐，可直接赋给 font.weight）─
    enum FontWeight {
        FontWeightNormal = 400,
        FontWeightMedium = 500,
        FontWeightBold   = 700
    };
    Q_ENUM(FontWeight)

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

    // ── 文字色 Token ──────────────────────────────────────────
    // primary: 正文/标题，secondary: 辅助说明，disabled: 禁用态
    Q_PROPERTY(QColor textPrimary   READ textPrimary   NOTIFY styleChanged)
    Q_PROPERTY(QColor textSecondary READ textSecondary NOTIFY styleChanged)
    Q_PROPERTY(QColor textDisabled  READ textDisabled  NOTIFY styleChanged)
    Q_PROPERTY(QColor textOnAccent  READ textOnAccent  NOTIFY styleChanged)

    // ── 表面色 Token ──────────────────────────────────────────
    // surface:   卡片/输入框底色，surfaceHover: 悬停态
    // separator: 分隔线颜色
    Q_PROPERTY(QColor surface      READ surface      NOTIFY styleChanged)
    Q_PROPERTY(QColor surfaceHover READ surfaceHover NOTIFY styleChanged)
    Q_PROPERTY(QColor separator    READ separator    NOTIFY styleChanged)

    // ── 输入框色 Token ────────────────────────────────────────
    Q_PROPERTY(QColor inputBg      READ inputBg      NOTIFY styleChanged)
    Q_PROPERTY(QColor inputBorder  READ inputBorder  NOTIFY styleChanged)
    Q_PROPERTY(QColor inputFocus   READ inputFocus   NOTIFY styleChanged)
    Q_PROPERTY(QColor inputText    READ inputText    NOTIFY styleChanged)
    Q_PROPERTY(QColor inputPlaceholder READ inputPlaceholder NOTIFY styleChanged)

    // ── 尺寸 Token ────────────────────────────────────────────
    Q_PROPERTY(int buttonSize      READ buttonSize      NOTIFY styleChanged)
    Q_PROPERTY(int buttonRadius    READ buttonRadius    NOTIFY styleChanged)
    Q_PROPERTY(int inputHeight     READ inputHeight     NOTIFY styleChanged)
    Q_PROPERTY(int inputRadius     READ inputRadius     NOTIFY styleChanged)
    Q_PROPERTY(int trackHeight     READ trackHeight     NOTIFY styleChanged)
    Q_PROPERTY(int handleSize      READ handleSize      NOTIFY styleChanged)

    // ── 字体 Token ────────────────────────────────────────────
    // fontFamily:      UI 字体族，空字符串表示使用系统默认字体
    // fontSize:        正文字号（px）
    // fontSizeLabel:   标签/次要文本字号（px）
    // fontSizeCaption: 说明文字字号（px）
    // fontWeightNormal/Medium/Bold: 对应 QFont::Weight 整数值，
    //   可直接赋给 QML 的 font.weight 属性
    Q_PROPERTY(QString fontFamily      READ fontFamily      NOTIFY styleChanged)
    Q_PROPERTY(int     fontSize        READ fontSize        NOTIFY styleChanged)
    Q_PROPERTY(int     fontSizeLabel   READ fontSizeLabel   NOTIFY styleChanged)
    Q_PROPERTY(int     fontSizeCaption READ fontSizeCaption NOTIFY styleChanged)
    Q_PROPERTY(int     fontWeightNormal READ fontWeightNormal CONSTANT)
    Q_PROPERTY(int     fontWeightMedium READ fontWeightMedium CONSTANT)
    Q_PROPERTY(int     fontWeightBold   READ fontWeightBold   CONSTANT)

    // ── 动画时长 Token ────────────────────────────────────────
    Q_PROPERTY(int  durationFast    READ durationFast    NOTIFY styleChanged)
    Q_PROPERTY(int  durationNormal  READ durationNormal  NOTIFY styleChanged)
    Q_PROPERTY(bool reducedMotion   READ reducedMotion   WRITE setReducedMotion NOTIFY styleChanged)

    // ── 色彩访问器 ────────────────────────────────────────────
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
    QColor textPrimary()     const { return m_textPrimary; }
    QColor textSecondary()   const { return m_textSecondary; }
    QColor textDisabled()    const { return m_textDisabled; }
    QColor textOnAccent()    const { return m_textOnAccent; }
    QColor surface()         const { return m_surface; }
    QColor surfaceHover()    const { return m_surfaceHover; }
    QColor separator()       const { return m_separator; }
    QColor inputBg()         const { return m_inputBg; }
    QColor inputBorder()     const { return m_inputBorder; }
    QColor inputFocus()      const { return m_inputFocus; }
    QColor inputText()       const { return m_inputText; }
    QColor inputPlaceholder() const { return m_inputPlaceholder; }

    // ── 尺寸访问器 ────────────────────────────────────────────
    int buttonSize()   const { return m_buttonSize; }
    int buttonRadius() const { return m_buttonRadius; }
    int inputHeight()  const { return m_inputHeight; }
    int inputRadius()  const { return m_inputRadius; }
    int trackHeight()  const { return m_trackHeight; }
    int handleSize()   const { return m_handleSize; }

    // ── 字体访问器 ────────────────────────────────────────────
    QString fontFamily()      const { return m_fontFamily; }
    int     fontSize()        const { return m_fontSize; }
    int     fontSizeLabel()   const { return m_fontSizeLabel; }
    int     fontSizeCaption() const { return m_fontSizeCaption; }
    int     fontWeightNormal() const { return FontWeightNormal; }
    int     fontWeightMedium() const { return FontWeightMedium; }
    int     fontWeightBold()   const { return FontWeightBold; }

    // ── 动画时长 ──────────────────────────────────────────────
    int  durationFast()   const { return m_reducedMotion ? 0 : m_durationFast; }
    int  durationNormal() const { return m_reducedMotion ? 0 : m_durationNormal; }
    bool reducedMotion()  const { return m_reducedMotion; }
    void setReducedMotion(bool reduced);

    // ── Custom 风格下允许逐项覆盖 ─────────────────────────────
    Q_INVOKABLE void setAccent(const QColor& c);
    Q_INVOKABLE void setButtonRadius(int r);
    Q_INVOKABLE void setFontFamily(const QString& family);

signals:
    void styleChanged();

private:
    explicit ComponentTheme(QObject* parent = nullptr);
    void applyDark();
    void applyLight();
    void applyDefaultSizes();   // Dark/Light 共用尺寸/字体，避免重复

    Style  m_style = Dark;

    // 强调色
    QColor m_accent;
    QColor m_accentHover;
    QColor m_accentPressed;
    QColor m_accentDisabled;
    // 图标色
    QColor m_iconColor;
    QColor m_iconColorPressed;
    // 按钮背景
    QColor m_buttonHover;
    QColor m_buttonPressed;
    // 滑轨
    QColor m_trackBg;
    QColor m_trackBuffer;
    QColor m_handleBorder;
    // 文字色
    QColor m_textPrimary;
    QColor m_textSecondary;
    QColor m_textDisabled;
    QColor m_textOnAccent;
    // 表面色
    QColor m_surface;
    QColor m_surfaceHover;
    QColor m_separator;
    // 输入框色
    QColor m_inputBg;
    QColor m_inputBorder;
    QColor m_inputFocus;
    QColor m_inputText;
    QColor m_inputPlaceholder;

    // 尺寸
    int m_buttonSize   = 34;
    int m_buttonRadius = 6;
    int m_inputHeight  = 36;
    int m_inputRadius  = 6;
    int m_trackHeight  = 4;
    int m_handleSize   = 14;

    // 字体
    QString m_fontFamily;        // 空串 = 系统默认
    int     m_fontSize        = 16;
    int     m_fontSizeLabel   = 13;
    int     m_fontSizeCaption = 11;

    // 动画时长
    int  m_durationFast   = 80;
    int  m_durationNormal = 120;
    bool m_reducedMotion  = false;
};
