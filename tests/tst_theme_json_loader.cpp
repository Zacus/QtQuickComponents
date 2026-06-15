#include <QtTest/QtTest>

#include <QColor>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "ThemeJsonLoader.h"

class ThemeJsonLoaderTest : public QObject
{
    Q_OBJECT

private slots:
    void loadsCompleteJson();
    void parsesCssRgbaHexColors();
    void rejectsOversizedInteger();
    void acceptsZeroRadius();
    void rejectsInvalidColor();
    void builtInThemeJsonResourcesAreAvailable();
    void loadsBuiltInDarkTheme();
    void loadsBuiltInLightTheme();
    void ignoresCurrentWorkingDirectoryThemes();
    void rejectsMismatchedRuntimeBuiltInTheme();
    void rejectsUnknownBuiltInTheme();

private:
    static QString writeFile(const QString& directory, const QString& name, const QString& content)
    {
        const QString path = QDir(directory).filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return {};
        file.write(content.toUtf8());
        return path;
    }

    static QString validThemeJson(int buttonRadius = 6, int inputRadius = 6)
    {
        return QStringLiteral(R"json({
  "id": "enterprise",
  "name": "Enterprise",
  "colors": {
    "accent": "#102030",
    "accentHover": "#223344",
    "accentPressed": "#334455",
    "accentDisabled": "#445566",
    "iconColor": "#556677",
    "iconColorPressed": "#667788",
    "buttonHover": "#778899",
    "buttonPressed": "#8899aa",
    "trackBg": "#99aabb",
    "trackBuffer": "#aabbcc",
    "handleBorder": "#bbccdd",
    "textPrimary": "#ccddee",
    "textSecondary": "#ddeeff",
    "textDisabled": "#123456",
    "textOnAccent": "#ffffff",
    "surface": "#101112",
    "surfaceHover": "#131415",
    "separator": "#161718",
    "inputBg": "#191a1b",
    "inputBorder": "#1c1d1e",
    "inputFocus": "#1f2021",
    "inputText": "#222324",
    "inputPlaceholder": "#252627"
  },
  "sizes": {
    "buttonSize": 34,
    "buttonRadius": %1,
    "inputHeight": 36,
    "inputRadius": %2,
    "trackHeight": 4,
    "handleSize": 14
  },
  "fonts": {
    "fontFamily": "",
    "fontSize": 16,
    "fontSizeLabel": 13,
    "fontSizeCaption": 11
  },
  "motion": {
    "durationFast": 80,
    "durationNormal": 120,
    "reducedMotion": false
  }
})json").arg(buttonRadius).arg(inputRadius);
    }
};

void ThemeJsonLoaderTest::loadsCompleteJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeFile(dir.path(), QStringLiteral("enterprise.json"), validThemeJson());
    QVERIFY(!path.isEmpty());

    ThemeLoadResult result = ThemeJsonLoader::loadFile(path);

    QVERIFY(result.ok);
    QCOMPARE(result.tokens.id, QStringLiteral("enterprise"));
    QCOMPARE(result.tokens.name, QStringLiteral("Enterprise"));
    QCOMPARE(result.tokens.accent, QColor(QStringLiteral("#102030")));
    QCOMPARE(result.tokens.buttonRadius, 6);
    QCOMPARE(result.error, QString());
}

void ThemeJsonLoaderTest::parsesCssRgbaHexColors()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString json = validThemeJson();
    json.replace(QStringLiteral("\"buttonHover\": \"#778899\""), QStringLiteral("\"buttonHover\": \"#11223344\""));
    const QString path = writeFile(dir.path(), QStringLiteral("rgba.json"), json);
    QVERIFY(!path.isEmpty());

    ThemeLoadResult result = ThemeJsonLoader::loadFile(path);

    QVERIFY(result.ok);
    QCOMPARE(result.tokens.buttonHover.red(), 0x11);
    QCOMPARE(result.tokens.buttonHover.green(), 0x22);
    QCOMPARE(result.tokens.buttonHover.blue(), 0x33);
    QCOMPARE(result.tokens.buttonHover.alpha(), 0x44);
}

void ThemeJsonLoaderTest::rejectsOversizedInteger()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString json = validThemeJson();
    json.replace(QStringLiteral("\"buttonSize\": 34"), QStringLiteral("\"buttonSize\": 2147483648"));
    const QString path = writeFile(dir.path(), QStringLiteral("bad.json"), json);
    QVERIFY(!path.isEmpty());

    ThemeLoadResult result = ThemeJsonLoader::loadFile(path);

    QVERIFY(!result.ok);
    QVERIFY(result.error.contains(QStringLiteral("buttonSize")));
}

void ThemeJsonLoaderTest::acceptsZeroRadius()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeFile(dir.path(), QStringLiteral("sharp.json"), validThemeJson(0, 0));
    QVERIFY(!path.isEmpty());

    ThemeLoadResult result = ThemeJsonLoader::loadFile(path);

    QVERIFY(result.ok);
    QCOMPARE(result.tokens.buttonRadius, 0);
    QCOMPARE(result.tokens.inputRadius, 0);
}

void ThemeJsonLoaderTest::rejectsInvalidColor()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QString json = validThemeJson();
    json.replace(QStringLiteral("\"accent\": \"#102030\""), QStringLiteral("\"accent\": \"not-a-color\""));
    const QString path = writeFile(dir.path(), QStringLiteral("bad-color.json"), json);
    QVERIFY(!path.isEmpty());

    ThemeLoadResult result = ThemeJsonLoader::loadFile(path);

    QVERIFY(!result.ok);
    QVERIFY(result.error.contains(QStringLiteral("accent")));
}

void ThemeJsonLoaderTest::builtInThemeJsonResourcesAreAvailable()
{
    QVERIFY(QFile::exists(QStringLiteral(":/QuickUI/Components/themes/dark.json")));
    QVERIFY(QFile::exists(QStringLiteral(":/QuickUI/Components/themes/light.json")));
}

void ThemeJsonLoaderTest::loadsBuiltInDarkTheme()
{
    ThemeLoadResult result = ThemeJsonLoader::loadBuiltInTheme(QStringLiteral("dark"));

    QVERIFY(result.ok);
    QCOMPARE(result.tokens.id, QStringLiteral("dark"));
    QCOMPARE(result.tokens.accent, QColor(QStringLiteral("#7c6fff")));
    QCOMPARE(result.error, QString());
}

void ThemeJsonLoaderTest::loadsBuiltInLightTheme()
{
    ThemeLoadResult result = ThemeJsonLoader::loadBuiltInTheme(QStringLiteral("light"));

    QVERIFY(result.ok);
    QCOMPARE(result.tokens.id, QStringLiteral("light"));
    QVERIFY(result.tokens.accent.isValid());
    QCOMPARE(result.error, QString());
}

void ThemeJsonLoaderTest::ignoresCurrentWorkingDirectoryThemes()
{
    struct CurrentPathRestorer
    {
        explicit CurrentPathRestorer(const QString& path)
            : originalPath(path)
        {
        }

        ~CurrentPathRestorer()
        {
            QDir::setCurrent(originalPath);
        }

        QString originalPath;
    };

    const QString originalPath = QDir::currentPath();
    CurrentPathRestorer restorer(originalPath);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath(QStringLiteral("themes")));

    QString shadowTheme = validThemeJson();
    shadowTheme.replace(QStringLiteral("\"id\": \"enterprise\""), QStringLiteral("\"id\": \"shadow-dark\""));
    shadowTheme.replace(QStringLiteral("\"name\": \"Enterprise\""), QStringLiteral("\"name\": \"Shadow Dark\""));
    shadowTheme.replace(QStringLiteral("\"accent\": \"#102030\""), QStringLiteral("\"accent\": \"#010203\""));
    const QString path = writeFile(QDir(dir.path()).filePath(QStringLiteral("themes")),
                                   QStringLiteral("dark.json"),
                                   shadowTheme);
    QVERIFY(!path.isEmpty());
    QVERIFY(QDir::setCurrent(dir.path()));

    ThemeLoadResult result = ThemeJsonLoader::loadBuiltInTheme(QStringLiteral("dark"));

    QVERIFY(result.ok);
    QCOMPARE(result.tokens.id, QStringLiteral("dark"));
    QCOMPARE(result.tokens.accent, QColor(QStringLiteral("#7c6fff")));
}

void ThemeJsonLoaderTest::rejectsMismatchedRuntimeBuiltInTheme()
{
    struct EnvironmentRestorer
    {
        explicit EnvironmentRestorer(const char* name)
            : variableName(name)
            , existed(qEnvironmentVariableIsSet(name))
            , originalValue(qgetenv(name))
        {
        }

        ~EnvironmentRestorer()
        {
            if (existed) {
                qputenv(variableName, originalValue);
            } else {
                qunsetenv(variableName);
            }
        }

        const char* variableName = nullptr;
        bool existed = false;
        QByteArray originalValue;
    };

    EnvironmentRestorer restorer("QTC_THEME_TEST_RUNTIME_DIR");
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    qputenv("QTC_THEME_TEST_RUNTIME_DIR", dir.path().toUtf8());

    QString mismatchedTheme = validThemeJson();
    mismatchedTheme.replace(QStringLiteral("\"id\": \"enterprise\""), QStringLiteral("\"id\": \"wrong-dark\""));
    mismatchedTheme.replace(QStringLiteral("\"name\": \"Enterprise\""), QStringLiteral("\"name\": \"Wrong Dark\""));
    const QString path = writeFile(dir.path(), QStringLiteral("dark.json"), mismatchedTheme);
    QVERIFY(!path.isEmpty());

    ThemeLoadResult result = ThemeJsonLoader::loadBuiltInTheme(QStringLiteral("dark"));

    QVERIFY(!result.ok);
    QVERIFY(result.error.contains(path));
    QVERIFY(result.error.contains(QStringLiteral("wrong-dark")));
}

void ThemeJsonLoaderTest::rejectsUnknownBuiltInTheme()
{
    ThemeLoadResult result = ThemeJsonLoader::loadBuiltInTheme(QStringLiteral("unknown"));

    QVERIFY(!result.ok);
    QVERIFY(result.error.contains(QStringLiteral("unknown")));
}

QTEST_MAIN(ThemeJsonLoaderTest)

#include "tst_theme_json_loader.moc"
