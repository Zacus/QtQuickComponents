#pragma once

#include "ThemeTokens.h"

#include <QByteArray>
#include <QString>
#include <QStringList>

struct ThemeLoadResult
{
    bool ok = false;
    ThemeTokens tokens;
    QString error;
};

class ThemeJsonLoader
{
public:
    static ThemeLoadResult loadFile(const QString& path);
    static ThemeLoadResult loadData(const QByteArray& data, const QString& sourceName);
    static ThemeLoadResult loadBuiltInTheme(const QString& themeId);
    static bool isValidThemeId(const QString& themeId);
    static QStringList builtInThemeIds();
};
