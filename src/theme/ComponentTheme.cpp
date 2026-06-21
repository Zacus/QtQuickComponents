#include "ComponentTheme.h"
#include "ThemeFileWatcher.h"
#include "ThemeJsonLoader.h"
#include "ThemeTokens.h"

#include <QDir>
#include <QFileInfo>

using QuickUI::Components::Internal::ThemeFileWatcher;
using QuickUI::Components::Internal::ThemeJsonLoader;
using QuickUI::Components::Internal::ThemeLoadResult;
using QuickUI::Components::Internal::ThemeTokens;

struct ComponentTheme::Private
{
    ThemeFileWatcher themeWatcher;
};

ComponentTheme::ComponentTheme(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    connect(&d->themeWatcher, &ThemeFileWatcher::reloadRequested,
            this, [this]() { reloadCurrentTheme(); });

    applyBuiltInTheme(QStringLiteral("dark"), Dark);
}

ComponentTheme::~ComponentTheme() = default;

void ComponentTheme::setStyle(Style s)
{
    if (m_style == s && m_currentThemeFile.isEmpty()) {
        if ((s == Dark && m_themeId == QStringLiteral("dark"))
            || (s == Light && m_themeId == QStringLiteral("light"))
            || s == Custom) {
            return;
        }
    }

    switch (s) {
    case Dark:
        if (!applyBuiltInTheme(QStringLiteral("dark"), Dark)) {
            return;
        }
        break;
    case Light:
        if (!applyBuiltInTheme(QStringLiteral("light"), Light)) {
            return;
        }
        break;
    case Custom:
        m_style = Custom;
        m_themeId = QStringLiteral("custom");
        m_themeName = QStringLiteral("Custom");
        m_currentThemeFile.clear();
        d->themeWatcher.clear();
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
        d->themeWatcher.setWatchedFile(m_currentThemeFile);
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
        d->themeWatcher.setEnabled(true);
        d->themeWatcher.setWatchedFile(m_currentThemeFile);
    } else {
        d->themeWatcher.setEnabled(false);
        d->themeWatcher.clear();
    }
    emit hotReloadEnabledChanged();
}

void ComponentTheme::setAccent(const QColor& c)
{
    m_style          = Custom;
    m_themeId        = QStringLiteral("custom");
    m_themeName      = QStringLiteral("Custom");
    m_currentThemeFile.clear();
    d->themeWatcher.clear();
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
    d->themeWatcher.clear();
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
    d->themeWatcher.clear();
    setLastError(QString());
    m_fontFamily = family;
    emit styleChanged();
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
    d->themeWatcher.setEnabled(m_hotReloadEnabled);
    d->themeWatcher.setWatchedFile(m_currentThemeFile);
}

void ComponentTheme::setLastError(const QString& error)
{
    if (m_lastError == error) return;
    m_lastError = error;
    emit lastErrorChanged();
}
