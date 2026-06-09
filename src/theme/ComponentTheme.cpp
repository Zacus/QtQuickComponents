#include "ComponentTheme.h"
#include "ThemeJsonLoader.h"
#include "ThemeTokens.h"

#include <QDir>
#include <QFileInfo>

ComponentTheme::ComponentTheme(QObject* parent)
    : QObject(parent)
{
    connect(&m_themeWatcher, &ThemeFileWatcher::reloadRequested,
            this, [this]() { reloadCurrentTheme(); });

    applyDark();
}

void ComponentTheme::setStyle(Style s)
{
    if (m_style == s && m_currentThemeFile.isEmpty()) {
        if ((s == Dark && m_themeId == QStringLiteral("dark"))
            || (s == Light && m_themeId == QStringLiteral("light"))
            || s == Custom) {
            return;
        }
    }

    m_style = s;
    switch (s) {
    case Dark:   applyDark();  break;
    case Light:  applyLight(); break;
    case Custom:
        m_themeId = QStringLiteral("custom");
        m_themeName = QStringLiteral("Custom");
        m_currentThemeFile.clear();
        m_themeWatcher.clear();
        break;   // 保留当前值，由外部逐项覆盖
    }
    setLastError(QString());
    emit styleChanged();
}

void ComponentTheme::setThemeDirectory(const QString& directory)
{
    const QString normalized = directory.trimmed().isEmpty()
        ? QString()
        : QDir(directory).absolutePath();
    if (m_themeDirectory == normalized) return;

    m_themeDirectory = normalized;
    emit themeDirectoryChanged();
}

bool ComponentTheme::loadTheme(const QString& themeId)
{
    const QString normalizedId = themeId.trimmed();
    if (!ThemeJsonLoader::isValidThemeId(normalizedId)) {
        setLastError(tr("Invalid theme id '%1'").arg(themeId));
        return false;
    }

    if (!m_themeDirectory.isEmpty()) {
        const QString path = QDir(m_themeDirectory).filePath(normalizedId + QStringLiteral(".json"));
        if (QFileInfo::exists(path)) {
            return loadThemeFile(path);
        }
    }

    if (normalizedId == QStringLiteral("dark")) {
        if (!applyBuiltInTheme(normalizedId, Dark)) {
            return false;
        }
        emit styleChanged();
        return true;
    }

    if (normalizedId == QStringLiteral("light")) {
        if (!applyBuiltInTheme(normalizedId, Light)) {
            return false;
        }
        emit styleChanged();
        return true;
    }

    setLastError(tr("Theme '%1' was not found in '%2'")
                     .arg(normalizedId, m_themeDirectory));
    return false;
}

bool ComponentTheme::loadThemeFile(const QString& path)
{
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    const ThemeLoadResult result = ThemeJsonLoader::loadFile(absolutePath);
    if (!result.ok) {
        setLastError(result.error);
        return false;
    }

    applyTokens(result.tokens, Custom, absolutePath);
    emit styleChanged();
    return true;
}

bool ComponentTheme::reloadCurrentTheme()
{
    if (m_currentThemeFile.isEmpty()) {
        setLastError(tr("No current theme file to reload"));
        return false;
    }

    const ThemeLoadResult result = ThemeJsonLoader::loadFile(m_currentThemeFile);
    if (!result.ok) {
        setLastError(result.error);
        m_themeWatcher.setWatchedFile(m_currentThemeFile);
        return false;
    }

    applyTokens(result.tokens, Custom, m_currentThemeFile);
    emit styleChanged();
    return true;
}

QStringList ComponentTheme::availableThemes() const
{
    QDir dir(m_themeDirectory);
    if (m_themeDirectory.isEmpty() || !dir.exists()) {
        return {};
    }

    const QStringList files = dir.entryList({ QStringLiteral("*.json") },
                                            QDir::Files,
                                            QDir::Name);
    QStringList themes;
    themes.reserve(files.size());
    for (const QString& file : files) {
        themes.append(QFileInfo(file).completeBaseName());
    }
    return themes;
}

void ComponentTheme::setHotReloadEnabled(bool enabled)
{
    if (m_hotReloadEnabled == enabled) return;

    m_hotReloadEnabled = enabled;
    if (m_hotReloadEnabled) {
        m_themeWatcher.setEnabled(true);
        m_themeWatcher.setWatchedFile(m_currentThemeFile);
    } else {
        m_themeWatcher.setEnabled(false);
        m_themeWatcher.clear();
    }
    emit hotReloadEnabledChanged();
}

void ComponentTheme::applyDark()
{
    m_themeId = QStringLiteral("dark");
    m_themeName = QStringLiteral("Dark");
    m_currentThemeFile.clear();
    m_themeWatcher.clear();

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
    m_themeId = QStringLiteral("light");
    m_themeName = QStringLiteral("Light");
    m_currentThemeFile.clear();
    m_themeWatcher.clear();

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
    m_themeId        = QStringLiteral("custom");
    m_themeName      = QStringLiteral("Custom");
    m_currentThemeFile.clear();
    m_themeWatcher.clear();
    setLastError(QString());
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
    m_themeId      = QStringLiteral("custom");
    m_themeName    = QStringLiteral("Custom");
    m_currentThemeFile.clear();
    m_themeWatcher.clear();
    setLastError(QString());
    m_buttonRadius = r;
    // 输入框圆角与按钮保持一致
    m_inputRadius  = r;
    emit styleChanged();
}

void ComponentTheme::setFontFamily(const QString& family)
{
    m_style      = Custom;
    m_themeId    = QStringLiteral("custom");
    m_themeName  = QStringLiteral("Custom");
    m_currentThemeFile.clear();
    m_themeWatcher.clear();
    setLastError(QString());
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

    m_durationFast   = 80;
    m_durationNormal = 120;
    m_reducedMotion  = false;
}

void ComponentTheme::setReducedMotion(bool reduced)
{
    if (m_reducedMotion == reduced) return;
    m_reducedMotion = reduced;
    emit styleChanged();
}

bool ComponentTheme::applyBuiltInTheme(const QString& themeId, Style style)
{
    const ThemeLoadResult result = ThemeJsonLoader::loadBuiltInTheme(themeId);
    if (!result.ok) {
        setLastError(result.error);
        return false;
    }

    applyTokens(result.tokens, style, QString());
    return true;
}

void ComponentTheme::applyTokens(const ThemeTokens& tokens, Style style, const QString& currentThemeFile)
{
    m_style = style;
    m_themeId = tokens.id;
    m_themeName = tokens.name;
    m_currentThemeFile = currentThemeFile;

    m_accent = tokens.accent;
    m_accentHover = tokens.accentHover;
    m_accentPressed = tokens.accentPressed;
    m_accentDisabled = tokens.accentDisabled;
    m_iconColor = tokens.iconColor;
    m_iconColorPressed = tokens.iconColorPressed;
    m_buttonHover = tokens.buttonHover;
    m_buttonPressed = tokens.buttonPressed;
    m_trackBg = tokens.trackBg;
    m_trackBuffer = tokens.trackBuffer;
    m_handleBorder = tokens.handleBorder;
    m_textPrimary = tokens.textPrimary;
    m_textSecondary = tokens.textSecondary;
    m_textDisabled = tokens.textDisabled;
    m_textOnAccent = tokens.textOnAccent;
    m_surface = tokens.surface;
    m_surfaceHover = tokens.surfaceHover;
    m_separator = tokens.separator;
    m_inputBg = tokens.inputBg;
    m_inputBorder = tokens.inputBorder;
    m_inputFocus = tokens.inputFocus;
    m_inputText = tokens.inputText;
    m_inputPlaceholder = tokens.inputPlaceholder;

    m_buttonSize = tokens.buttonSize;
    m_buttonRadius = tokens.buttonRadius;
    m_inputHeight = tokens.inputHeight;
    m_inputRadius = tokens.inputRadius;
    m_trackHeight = tokens.trackHeight;
    m_handleSize = tokens.handleSize;

    m_fontFamily = tokens.fontFamily;
    m_fontSize = tokens.fontSize;
    m_fontSizeLabel = tokens.fontSizeLabel;
    m_fontSizeCaption = tokens.fontSizeCaption;

    m_durationFast = tokens.durationFast;
    m_durationNormal = tokens.durationNormal;
    m_reducedMotion = tokens.reducedMotion;

    setLastError(QString());
    m_themeWatcher.setEnabled(m_hotReloadEnabled);
    m_themeWatcher.setWatchedFile(m_currentThemeFile);
}

void ComponentTheme::setLastError(const QString& error)
{
    if (m_lastError == error) return;
    m_lastError = error;
    emit lastErrorChanged();
}
