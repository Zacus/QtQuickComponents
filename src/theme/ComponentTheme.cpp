#include "ComponentTheme.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>

#include <cmath>
#include <limits>

ComponentTheme::ComponentTheme(QObject* parent)
    : QObject(parent)
{
    m_reloadDebounceTimer.setSingleShot(true);
    m_reloadDebounceTimer.setInterval(80);
    m_themePollTimer.setInterval(100);

    connect(&m_themeWatcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString&) { scheduleThemeReload(); });
    connect(&m_themeWatcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString&) { scheduleThemeReload(); });
    connect(&m_reloadDebounceTimer, &QTimer::timeout,
            this, [this]() { reloadCurrentTheme(); });
    connect(&m_themePollTimer, &QTimer::timeout,
            this, [this]() {
                if (m_currentThemeFile.isEmpty()) {
                    return;
                }

                const QFileInfo info(m_currentThemeFile);
                if (!info.exists()) {
                    return;
                }

                const qint64 lastModifiedMs = info.lastModified().toMSecsSinceEpoch();
                if (m_currentThemeFileLastModifiedMs == 0) {
                    m_currentThemeFileLastModifiedMs = lastModifiedMs;
                    return;
                }
                if (lastModifiedMs != m_currentThemeFileLastModifiedMs) {
                    m_currentThemeFileLastModifiedMs = lastModifiedMs;
                    scheduleThemeReload();
                }
            });

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
        clearThemeWatcher();
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
    if (!isValidThemeId(normalizedId)) {
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
        m_style = Dark;
        applyDark();
        setLastError(QString());
        emit styleChanged();
        return true;
    }

    if (normalizedId == QStringLiteral("light")) {
        m_style = Light;
        applyLight();
        setLastError(QString());
        emit styleChanged();
        return true;
    }

    setLastError(tr("Theme '%1' was not found in '%2'")
                     .arg(normalizedId, m_themeDirectory));
    return false;
}

bool ComponentTheme::loadThemeFile(const QString& path)
{
    ThemeTokens tokens;
    QString error;
    const QString absolutePath = QFileInfo(path).absoluteFilePath();
    if (!parseThemeFile(absolutePath, tokens, error)) {
        setLastError(error);
        return false;
    }

    applyTokens(tokens, Custom, absolutePath);
    emit styleChanged();
    return true;
}

bool ComponentTheme::reloadCurrentTheme()
{
    if (m_currentThemeFile.isEmpty()) {
        setLastError(tr("No current theme file to reload"));
        return false;
    }

    ThemeTokens tokens;
    QString error;
    if (!parseThemeFile(m_currentThemeFile, tokens, error)) {
        setLastError(error);
        configureThemeWatcher();
        return false;
    }

    applyTokens(tokens, Custom, m_currentThemeFile);
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
        configureThemeWatcher();
    } else {
        clearThemeWatcher();
        m_reloadDebounceTimer.stop();
    }
    emit hotReloadEnabledChanged();
}

void ComponentTheme::applyDark()
{
    m_themeId = QStringLiteral("dark");
    m_themeName = QStringLiteral("Dark");
    m_currentThemeFile.clear();
    clearThemeWatcher();

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
    clearThemeWatcher();

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
    clearThemeWatcher();
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
    clearThemeWatcher();
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
    clearThemeWatcher();
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
    configureThemeWatcher();
}

bool ComponentTheme::parseThemeFile(const QString& path, ThemeTokens& tokens, QString& error) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        error = tr("Failed to open theme file '%1': %2").arg(path, file.errorString());
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        error = tr("Failed to parse theme file '%1': %2 at offset %3")
                    .arg(path, parseError.errorString())
                    .arg(parseError.offset);
        return false;
    }

    if (!document.isObject()) {
        error = tr("Theme file '%1' must contain a JSON object").arg(path);
        return false;
    }

    const QJsonObject root = document.object();
    ThemeTokens parsed;

    const auto requiredObject = [&](const QJsonObject& object,
                                    const QString& key,
                                    QJsonObject& value) -> bool {
        const QJsonValue jsonValue = object.value(key);
        if (!jsonValue.isObject()) {
            error = tr("Theme file '%1' is missing object '%2'").arg(path, key);
            return false;
        }
        value = jsonValue.toObject();
        return true;
    };

    const auto requiredString = [&](const QJsonObject& object,
                                    const QString& key,
                                    QString& value,
                                    bool allowEmpty = false) -> bool {
        const QJsonValue jsonValue = object.value(key);
        if (!jsonValue.isString()) {
            error = tr("Theme file '%1' is missing string '%2'").arg(path, key);
            return false;
        }
        value = jsonValue.toString();
        if (!allowEmpty && value.isEmpty()) {
            error = tr("Theme file '%1' has empty string '%2'").arg(path, key);
            return false;
        }
        return true;
    };

    const auto requiredColor = [&](const QJsonObject& object,
                                   const QString& key,
                                   QColor& value) -> bool {
        QString colorText;
        if (!requiredString(object, key, colorText)) {
            return false;
        }
        const QColor color(colorText);
        if (!color.isValid()) {
            error = tr("Theme file '%1' has invalid color '%2' for '%3'")
                        .arg(path, colorText, key);
            return false;
        }
        value = color;
        return true;
    };

    const auto requiredInt = [&](const QJsonObject& object,
                                 const QString& key,
                                 int minimum,
                                 int& value) -> bool {
        const QJsonValue jsonValue = object.value(key);
        if (!jsonValue.isDouble()) {
            error = tr("Theme file '%1' is missing number '%2'").arg(path, key);
            return false;
        }
        const double number = jsonValue.toDouble();
        if (number < minimum
            || number > std::numeric_limits<int>::max()
            || std::floor(number) != number) {
            error = tr("Theme file '%1' has invalid number '%2'").arg(path, key);
            return false;
        }
        value = int(number);
        return true;
    };

    const auto requiredBool = [&](const QJsonObject& object,
                                  const QString& key,
                                  bool& value) -> bool {
        const QJsonValue jsonValue = object.value(key);
        if (!jsonValue.isBool()) {
            error = tr("Theme file '%1' is missing boolean '%2'").arg(path, key);
            return false;
        }
        value = jsonValue.toBool();
        return true;
    };

    if (!requiredString(root, QStringLiteral("id"), parsed.id)) return false;
    if (!isValidThemeId(parsed.id)) {
        error = tr("Theme file '%1' has invalid theme id '%2'").arg(path, parsed.id);
        return false;
    }
    if (!requiredString(root, QStringLiteral("name"), parsed.name)) return false;

    QJsonObject colors;
    QJsonObject sizes;
    QJsonObject fonts;
    QJsonObject motion;
    if (!requiredObject(root, QStringLiteral("colors"), colors)) return false;
    if (!requiredObject(root, QStringLiteral("sizes"), sizes)) return false;
    if (!requiredObject(root, QStringLiteral("fonts"), fonts)) return false;
    if (!requiredObject(root, QStringLiteral("motion"), motion)) return false;

    if (!requiredColor(colors, QStringLiteral("accent"), parsed.accent)) return false;
    if (!requiredColor(colors, QStringLiteral("accentHover"), parsed.accentHover)) return false;
    if (!requiredColor(colors, QStringLiteral("accentPressed"), parsed.accentPressed)) return false;
    if (!requiredColor(colors, QStringLiteral("accentDisabled"), parsed.accentDisabled)) return false;
    if (!requiredColor(colors, QStringLiteral("iconColor"), parsed.iconColor)) return false;
    if (!requiredColor(colors, QStringLiteral("iconColorPressed"), parsed.iconColorPressed)) return false;
    if (!requiredColor(colors, QStringLiteral("buttonHover"), parsed.buttonHover)) return false;
    if (!requiredColor(colors, QStringLiteral("buttonPressed"), parsed.buttonPressed)) return false;
    if (!requiredColor(colors, QStringLiteral("trackBg"), parsed.trackBg)) return false;
    if (!requiredColor(colors, QStringLiteral("trackBuffer"), parsed.trackBuffer)) return false;
    if (!requiredColor(colors, QStringLiteral("handleBorder"), parsed.handleBorder)) return false;
    if (!requiredColor(colors, QStringLiteral("textPrimary"), parsed.textPrimary)) return false;
    if (!requiredColor(colors, QStringLiteral("textSecondary"), parsed.textSecondary)) return false;
    if (!requiredColor(colors, QStringLiteral("textDisabled"), parsed.textDisabled)) return false;
    if (!requiredColor(colors, QStringLiteral("textOnAccent"), parsed.textOnAccent)) return false;
    if (!requiredColor(colors, QStringLiteral("surface"), parsed.surface)) return false;
    if (!requiredColor(colors, QStringLiteral("surfaceHover"), parsed.surfaceHover)) return false;
    if (!requiredColor(colors, QStringLiteral("separator"), parsed.separator)) return false;
    if (!requiredColor(colors, QStringLiteral("inputBg"), parsed.inputBg)) return false;
    if (!requiredColor(colors, QStringLiteral("inputBorder"), parsed.inputBorder)) return false;
    if (!requiredColor(colors, QStringLiteral("inputFocus"), parsed.inputFocus)) return false;
    if (!requiredColor(colors, QStringLiteral("inputText"), parsed.inputText)) return false;
    if (!requiredColor(colors, QStringLiteral("inputPlaceholder"), parsed.inputPlaceholder)) return false;

    if (!requiredInt(sizes, QStringLiteral("buttonSize"), 1, parsed.buttonSize)) return false;
    if (!requiredInt(sizes, QStringLiteral("buttonRadius"), 0, parsed.buttonRadius)) return false;
    if (!requiredInt(sizes, QStringLiteral("inputHeight"), 1, parsed.inputHeight)) return false;
    if (!requiredInt(sizes, QStringLiteral("inputRadius"), 0, parsed.inputRadius)) return false;
    if (!requiredInt(sizes, QStringLiteral("trackHeight"), 1, parsed.trackHeight)) return false;
    if (!requiredInt(sizes, QStringLiteral("handleSize"), 1, parsed.handleSize)) return false;

    if (!requiredString(fonts, QStringLiteral("fontFamily"), parsed.fontFamily, true)) return false;
    if (!requiredInt(fonts, QStringLiteral("fontSize"), 1, parsed.fontSize)) return false;
    if (!requiredInt(fonts, QStringLiteral("fontSizeLabel"), 1, parsed.fontSizeLabel)) return false;
    if (!requiredInt(fonts, QStringLiteral("fontSizeCaption"), 1, parsed.fontSizeCaption)) return false;

    if (!requiredInt(motion, QStringLiteral("durationFast"), 0, parsed.durationFast)) return false;
    if (!requiredInt(motion, QStringLiteral("durationNormal"), 0, parsed.durationNormal)) return false;
    if (!requiredBool(motion, QStringLiteral("reducedMotion"), parsed.reducedMotion)) return false;

    tokens = parsed;
    return true;
}

void ComponentTheme::setLastError(const QString& error)
{
    if (m_lastError == error) return;
    m_lastError = error;
    emit lastErrorChanged();
}

void ComponentTheme::clearThemeWatcher()
{
    m_reloadDebounceTimer.stop();
    const QStringList files = m_themeWatcher.files();
    if (!files.isEmpty()) {
        m_themeWatcher.removePaths(files);
    }
    const QStringList directories = m_themeWatcher.directories();
    if (!directories.isEmpty()) {
        m_themeWatcher.removePaths(directories);
    }
    m_themePollTimer.stop();
    m_currentThemeFileLastModifiedMs = 0;
}

void ComponentTheme::configureThemeWatcher()
{
    clearThemeWatcher();
    if (!m_hotReloadEnabled || m_currentThemeFile.isEmpty()) {
        return;
    }

    const QFileInfo info(m_currentThemeFile);
    if (info.exists()) {
        m_currentThemeFileLastModifiedMs = info.lastModified().toMSecsSinceEpoch();
        const bool watchesFile = m_themeWatcher.addPath(info.absoluteFilePath());
        const bool watchesDirectory = m_themeWatcher.addPath(info.absolutePath());
        if (!watchesFile && !watchesDirectory) {
            m_themePollTimer.start();
        }
    }
}

void ComponentTheme::scheduleThemeReload()
{
    if (!m_hotReloadEnabled) {
        return;
    }

    m_reloadDebounceTimer.start();
}

bool ComponentTheme::isValidThemeId(const QString& themeId)
{
    static const QRegularExpression expression(QStringLiteral("^[A-Za-z0-9_-]+$"));
    return !themeId.isEmpty() && expression.match(themeId).hasMatch();
}
