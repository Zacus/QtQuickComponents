#include "ThemeJsonLoader.h"

#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QRegularExpression>

#include <cmath>
#include <limits>

#ifndef QTC_THEME_SOURCE_DIR
#define QTC_THEME_SOURCE_DIR ""
#endif

#ifndef QTC_THEME_BUILD_DIR
#define QTC_THEME_BUILD_DIR ""
#endif

#ifndef QTC_THEME_INSTALL_DIR
#define QTC_THEME_INSTALL_DIR ""
#endif

namespace {

QStringList builtInThemeCandidatePaths(const QString& themeId)
{
    const QString fileName = themeId + QStringLiteral(".json");
    const QString applicationDir = QCoreApplication::applicationDirPath();

    QStringList paths;
    const auto appendThemePath = [&](const QString& directory) {
        if (directory.isEmpty()) {
            return;
        }
        const QString path = QDir::cleanPath(QDir(directory).filePath(fileName));
        if (!paths.contains(path)) {
            paths.append(path);
        }
    };

#ifdef QTC_ENABLE_THEME_TEST_OVERRIDES
    appendThemePath(qEnvironmentVariable("QTC_THEME_TEST_RUNTIME_DIR"));
#endif
    appendThemePath(QDir(applicationDir).filePath(QStringLiteral("themes")));
    appendThemePath(QDir(applicationDir).filePath(QStringLiteral("../themes")));
    appendThemePath(QDir(applicationDir).filePath(QStringLiteral("../Resources/themes")));
    appendThemePath(QDir(applicationDir).filePath(QStringLiteral("../share/QtQuickComponents/themes")));
    appendThemePath(QDir(applicationDir).filePath(QStringLiteral("../../share/QtQuickComponents/themes")));
    appendThemePath(QString::fromUtf8(QTC_THEME_INSTALL_DIR));
    appendThemePath(QString::fromUtf8(QTC_THEME_BUILD_DIR));
    appendThemePath(QString::fromUtf8(QTC_THEME_SOURCE_DIR));

    return paths;
}

} // namespace

ThemeLoadResult ThemeJsonLoader::loadFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return { false, {}, QStringLiteral("Failed to open theme file '%1': %2").arg(path, file.errorString()) };
    }

    return loadData(file.readAll(), path);
}

ThemeLoadResult ThemeJsonLoader::loadData(const QByteArray& data, const QString& sourceName)
{
    ThemeLoadResult result;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        result.error = QStringLiteral("Failed to parse theme file '%1': %2 at offset %3")
                           .arg(sourceName, parseError.errorString())
                           .arg(parseError.offset);
        return result;
    }

    if (!document.isObject()) {
        result.error = QStringLiteral("Theme file '%1' must contain a JSON object").arg(sourceName);
        return result;
    }

    const QJsonObject root = document.object();
    ThemeTokens parsed;

    const auto requiredObject = [&](const QJsonObject& object,
                                    const QString& key,
                                    QJsonObject& value) -> bool {
        const QJsonValue jsonValue = object.value(key);
        if (!jsonValue.isObject()) {
            result.error = QStringLiteral("Theme file '%1' is missing object '%2'").arg(sourceName, key);
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
            result.error = QStringLiteral("Theme file '%1' is missing string '%2'").arg(sourceName, key);
            return false;
        }
        value = jsonValue.toString();
        if (!allowEmpty && value.isEmpty()) {
            result.error = QStringLiteral("Theme file '%1' has empty string '%2'").arg(sourceName, key);
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
            result.error = QStringLiteral("Theme file '%1' has invalid color '%2' for '%3'")
                               .arg(sourceName, colorText, key);
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
            result.error = QStringLiteral("Theme file '%1' is missing number '%2'").arg(sourceName, key);
            return false;
        }
        const double number = jsonValue.toDouble();
        if (number < minimum
            || number > std::numeric_limits<int>::max()
            || std::floor(number) != number) {
            result.error = QStringLiteral("Theme file '%1' has invalid number '%2'").arg(sourceName, key);
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
            result.error = QStringLiteral("Theme file '%1' is missing boolean '%2'").arg(sourceName, key);
            return false;
        }
        value = jsonValue.toBool();
        return true;
    };

    if (!requiredString(root, QStringLiteral("id"), parsed.id)) return result;
    if (!isValidThemeId(parsed.id)) {
        result.error = QStringLiteral("Theme file '%1' has invalid theme id '%2'").arg(sourceName, parsed.id);
        return result;
    }
    if (!requiredString(root, QStringLiteral("name"), parsed.name)) return result;

    QJsonObject colors;
    QJsonObject sizes;
    QJsonObject fonts;
    QJsonObject motion;
    if (!requiredObject(root, QStringLiteral("colors"), colors)) return result;
    if (!requiredObject(root, QStringLiteral("sizes"), sizes)) return result;
    if (!requiredObject(root, QStringLiteral("fonts"), fonts)) return result;
    if (!requiredObject(root, QStringLiteral("motion"), motion)) return result;

    if (!requiredColor(colors, QStringLiteral("accent"), parsed.accent)) return result;
    if (!requiredColor(colors, QStringLiteral("accentHover"), parsed.accentHover)) return result;
    if (!requiredColor(colors, QStringLiteral("accentPressed"), parsed.accentPressed)) return result;
    if (!requiredColor(colors, QStringLiteral("accentDisabled"), parsed.accentDisabled)) return result;
    if (!requiredColor(colors, QStringLiteral("iconColor"), parsed.iconColor)) return result;
    if (!requiredColor(colors, QStringLiteral("iconColorPressed"), parsed.iconColorPressed)) return result;
    if (!requiredColor(colors, QStringLiteral("buttonHover"), parsed.buttonHover)) return result;
    if (!requiredColor(colors, QStringLiteral("buttonPressed"), parsed.buttonPressed)) return result;
    if (!requiredColor(colors, QStringLiteral("trackBg"), parsed.trackBg)) return result;
    if (!requiredColor(colors, QStringLiteral("trackBuffer"), parsed.trackBuffer)) return result;
    if (!requiredColor(colors, QStringLiteral("handleBorder"), parsed.handleBorder)) return result;
    if (!requiredColor(colors, QStringLiteral("textPrimary"), parsed.textPrimary)) return result;
    if (!requiredColor(colors, QStringLiteral("textSecondary"), parsed.textSecondary)) return result;
    if (!requiredColor(colors, QStringLiteral("textDisabled"), parsed.textDisabled)) return result;
    if (!requiredColor(colors, QStringLiteral("textOnAccent"), parsed.textOnAccent)) return result;
    if (!requiredColor(colors, QStringLiteral("surface"), parsed.surface)) return result;
    if (!requiredColor(colors, QStringLiteral("surfaceHover"), parsed.surfaceHover)) return result;
    if (!requiredColor(colors, QStringLiteral("separator"), parsed.separator)) return result;
    if (!requiredColor(colors, QStringLiteral("inputBg"), parsed.inputBg)) return result;
    if (!requiredColor(colors, QStringLiteral("inputBorder"), parsed.inputBorder)) return result;
    if (!requiredColor(colors, QStringLiteral("inputFocus"), parsed.inputFocus)) return result;
    if (!requiredColor(colors, QStringLiteral("inputText"), parsed.inputText)) return result;
    if (!requiredColor(colors, QStringLiteral("inputPlaceholder"), parsed.inputPlaceholder)) return result;

    if (!requiredInt(sizes, QStringLiteral("buttonSize"), 1, parsed.buttonSize)) return result;
    if (!requiredInt(sizes, QStringLiteral("buttonRadius"), 0, parsed.buttonRadius)) return result;
    if (!requiredInt(sizes, QStringLiteral("inputHeight"), 1, parsed.inputHeight)) return result;
    if (!requiredInt(sizes, QStringLiteral("inputRadius"), 0, parsed.inputRadius)) return result;
    if (!requiredInt(sizes, QStringLiteral("trackHeight"), 1, parsed.trackHeight)) return result;
    if (!requiredInt(sizes, QStringLiteral("handleSize"), 1, parsed.handleSize)) return result;

    if (!requiredString(fonts, QStringLiteral("fontFamily"), parsed.fontFamily, true)) return result;
    if (!requiredInt(fonts, QStringLiteral("fontSize"), 1, parsed.fontSize)) return result;
    if (!requiredInt(fonts, QStringLiteral("fontSizeLabel"), 1, parsed.fontSizeLabel)) return result;
    if (!requiredInt(fonts, QStringLiteral("fontSizeCaption"), 1, parsed.fontSizeCaption)) return result;

    if (!requiredInt(motion, QStringLiteral("durationFast"), 0, parsed.durationFast)) return result;
    if (!requiredInt(motion, QStringLiteral("durationNormal"), 0, parsed.durationNormal)) return result;
    if (!requiredBool(motion, QStringLiteral("reducedMotion"), parsed.reducedMotion)) return result;

    result.ok = true;
    result.tokens = parsed;
    result.error.clear();
    return result;
}

ThemeLoadResult ThemeJsonLoader::loadBuiltInTheme(const QString& themeId)
{
    const QString normalizedId = themeId.trimmed();
    if (!builtInThemeIds().contains(normalizedId)) {
        return { false, {}, QStringLiteral("Theme '%1' is not a built-in theme").arg(themeId) };
    }

    const QStringList paths = builtInThemeCandidatePaths(normalizedId);
    for (const QString& path : paths) {
        if (!QFileInfo::exists(path)) {
            continue;
        }

        ThemeLoadResult result = loadFile(path);
        if (!result.ok) {
            return {
                false,
                {},
                QStringLiteral("Failed to load built-in theme '%1' from '%2': %3")
                    .arg(normalizedId, path, result.error)
            };
        }

        if (result.tokens.id == normalizedId) {
            return result;
        }

        return {
            false,
            {},
            QStringLiteral("Built-in theme '%1' from '%2' has mismatched id '%3'")
                .arg(normalizedId, path, result.tokens.id)
        };
    }

    return { false, {}, QStringLiteral("Built-in theme '%1' was not found").arg(normalizedId) };
}

bool ThemeJsonLoader::isValidThemeId(const QString& themeId)
{
    static const QRegularExpression expression(QStringLiteral("^[A-Za-z0-9_-]+$"));
    return !themeId.isEmpty() && expression.match(themeId).hasMatch();
}

QStringList ThemeJsonLoader::builtInThemeIds()
{
    return { QStringLiteral("dark"), QStringLiteral("light") };
}
