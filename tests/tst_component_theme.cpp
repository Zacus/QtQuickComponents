#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "ComponentTheme.h"

class ComponentThemeTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void loadThemeFileLoadsCompleteJson();
    void invalidJsonDoesNotPolluteExistingTokens();
    void invalidColorDoesNotPolluteExistingTokens();
    void oversizedIntegerDoesNotPolluteExistingTokens();
    void zeroRadiusIsValidThemeToken();
    void loadThemeUsesThemeDirectoryJson();
    void availableThemesListsJsonFileIds();
    void hotReloadUpdatesCurrentTheme();
    void hotReloadSurvivesRepeatedAtomicRewrites();

private:
    static QString themeJson(const QString& id,
                             const QString& name,
                             const QString& accent,
                             int buttonSize,
                             const QString& fontFamily = QStringLiteral("Inter"),
                             int buttonRadius = 7,
                             int inputRadius = 8);
    static QString writeThemeFile(const QString& directory,
                                  const QString& fileName,
                                  const QString& contents);
    static QString atomicReplaceThemeFile(const QString& directory,
                                          const QString& fileName,
                                          const QString& contents);
};

void ComponentThemeTest::init()
{
    ComponentTheme& theme = ComponentTheme::instance();
    theme.setHotReloadEnabled(false);
    theme.setThemeDirectory(QString());
    theme.setStyle(ComponentTheme::Dark);
}

void ComponentThemeTest::cleanup()
{
    ComponentTheme& theme = ComponentTheme::instance();
    theme.setHotReloadEnabled(false);
    theme.setThemeDirectory(QString());
    theme.setStyle(ComponentTheme::Dark);
}

QString ComponentThemeTest::themeJson(const QString& id,
                                      const QString& name,
                                      const QString& accent,
                                      int buttonSize,
                                      const QString& fontFamily,
                                      int buttonRadius,
                                      int inputRadius)
{
    return QStringLiteral(R"json({
  "id": "%1",
  "name": "%2",
  "colors": {
    "accent": "%3",
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
    "buttonSize": %4,
    "buttonRadius": %6,
    "inputHeight": 38,
    "inputRadius": %7,
    "trackHeight": 5,
    "handleSize": 16
  },
  "fonts": {
    "fontFamily": "%5",
    "fontSize": 17,
    "fontSizeLabel": 14,
    "fontSizeCaption": 12
  },
  "motion": {
    "durationFast": 90,
    "durationNormal": 140,
    "reducedMotion": false
  }
}
)json")
        .arg(id, name, accent)
        .arg(buttonSize)
        .arg(fontFamily)
        .arg(buttonRadius)
        .arg(inputRadius);
}

QString ComponentThemeTest::writeThemeFile(const QString& directory,
                                           const QString& fileName,
                                           const QString& contents)
{
    const QString path = QDir(directory).filePath(fileName);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return QString();
    }
    file.write(contents.toUtf8());
    file.close();
    return path;
}

QString ComponentThemeTest::atomicReplaceThemeFile(const QString& directory,
                                                   const QString& fileName,
                                                   const QString& contents)
{
    const QString path = QDir(directory).filePath(fileName);
    const QString tempPath = QDir(directory).filePath(fileName + QStringLiteral(".tmp"));
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return QString();
    }
    tempFile.write(contents.toUtf8());
    tempFile.close();
    QFile::remove(path);
    if (!QFile::rename(tempPath, path)) {
        QFile::remove(tempPath);
        return QString();
    }
    return path;
}

void ComponentThemeTest::loadThemeFileLoadsCompleteJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeThemeFile(dir.path(),
                                        QStringLiteral("ocean.json"),
                                        themeJson(QStringLiteral("ocean"),
                                                  QStringLiteral("Ocean"),
                                                  QStringLiteral("#102030"),
                                                  44));
    QVERIFY(!path.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();

    QVERIFY(theme.loadThemeFile(path));
    QCOMPARE(theme.style(), ComponentTheme::Custom);
    QCOMPARE(theme.themeId(), QStringLiteral("ocean"));
    QCOMPARE(theme.themeName(), QStringLiteral("Ocean"));
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#102030")));
    QCOMPARE(theme.buttonSize(), 44);
    QCOMPARE(theme.buttonRadius(), 7);
    QCOMPARE(theme.fontFamily(), QStringLiteral("Inter"));
    QCOMPARE(theme.fontSize(), 17);
    QCOMPARE(theme.durationFast(), 90);
    QCOMPARE(theme.durationNormal(), 140);
    QCOMPARE(theme.lastError(), QString());
}

void ComponentThemeTest::invalidJsonDoesNotPolluteExistingTokens()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString goodPath = writeThemeFile(dir.path(),
                                           QStringLiteral("good.json"),
                                           themeJson(QStringLiteral("good"),
                                                     QStringLiteral("Good"),
                                                     QStringLiteral("#102030"),
                                                     44));
    const QString badPath = writeThemeFile(dir.path(),
                                          QStringLiteral("bad.json"),
                                          QStringLiteral("{ \"id\": \"bad\", "));
    QVERIFY(!goodPath.isEmpty());
    QVERIFY(!badPath.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    QVERIFY(theme.loadThemeFile(goodPath));
    const QColor oldAccent = theme.accent();
    const int oldButtonSize = theme.buttonSize();
    const QString oldThemeId = theme.themeId();

    QVERIFY(!theme.loadThemeFile(badPath));
    QCOMPARE(theme.accent(), oldAccent);
    QCOMPARE(theme.buttonSize(), oldButtonSize);
    QCOMPARE(theme.themeId(), oldThemeId);
    QVERIFY(!theme.lastError().isEmpty());
}

void ComponentThemeTest::invalidColorDoesNotPolluteExistingTokens()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString goodPath = writeThemeFile(dir.path(),
                                           QStringLiteral("good.json"),
                                           themeJson(QStringLiteral("good"),
                                                     QStringLiteral("Good"),
                                                     QStringLiteral("#102030"),
                                                     44));
    const QString badPath = writeThemeFile(dir.path(),
                                          QStringLiteral("bad-color.json"),
                                          themeJson(QStringLiteral("bad-color"),
                                                    QStringLiteral("Bad Color"),
                                                    QStringLiteral("not-a-color"),
                                                    52));
    QVERIFY(!goodPath.isEmpty());
    QVERIFY(!badPath.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    QVERIFY(theme.loadThemeFile(goodPath));
    const QColor oldAccent = theme.accent();
    const int oldButtonSize = theme.buttonSize();
    const QString oldThemeId = theme.themeId();

    QVERIFY(!theme.loadThemeFile(badPath));
    QCOMPARE(theme.accent(), oldAccent);
    QCOMPARE(theme.buttonSize(), oldButtonSize);
    QCOMPARE(theme.themeId(), oldThemeId);
    QVERIFY(!theme.lastError().isEmpty());
}

void ComponentThemeTest::oversizedIntegerDoesNotPolluteExistingTokens()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString goodPath = writeThemeFile(dir.path(),
                                           QStringLiteral("good.json"),
                                           themeJson(QStringLiteral("good"),
                                                     QStringLiteral("Good"),
                                                     QStringLiteral("#102030"),
                                                     44));
    QString oversized = themeJson(QStringLiteral("oversized"),
                                  QStringLiteral("Oversized"),
                                  QStringLiteral("#203040"),
                                  45);
    oversized.replace(QStringLiteral("\"buttonSize\": 45"),
                      QStringLiteral("\"buttonSize\": 2147483648"));
    const QString badPath = writeThemeFile(dir.path(), QStringLiteral("oversized.json"), oversized);
    QVERIFY(!goodPath.isEmpty());
    QVERIFY(!badPath.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    QVERIFY(theme.loadThemeFile(goodPath));
    const int oldButtonSize = theme.buttonSize();
    const QString oldThemeId = theme.themeId();

    QVERIFY(!theme.loadThemeFile(badPath));
    QCOMPARE(theme.buttonSize(), oldButtonSize);
    QCOMPARE(theme.themeId(), oldThemeId);
    QVERIFY(!theme.lastError().isEmpty());
}

void ComponentThemeTest::zeroRadiusIsValidThemeToken()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeThemeFile(dir.path(),
                                        QStringLiteral("sharp.json"),
                                        themeJson(QStringLiteral("sharp"),
                                                  QStringLiteral("Sharp"),
                                                  QStringLiteral("#102030"),
                                                  44,
                                                  QStringLiteral("Inter"),
                                                  0,
                                                  0));
    QVERIFY(!path.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    QVERIFY(theme.loadThemeFile(path));
    QCOMPARE(theme.buttonRadius(), 0);
    QCOMPARE(theme.inputRadius(), 0);
}

void ComponentThemeTest::loadThemeUsesThemeDirectoryJson()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeThemeFile(dir.path(),
                                        QStringLiteral("dark.json"),
                                        themeJson(QStringLiteral("dark"),
                                                  QStringLiteral("Directory Dark"),
                                                  QStringLiteral("#123456"),
                                                  50));
    QVERIFY(!path.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    theme.setThemeDirectory(dir.path());

    QVERIFY(theme.loadTheme(QStringLiteral("dark")));
    QCOMPARE(theme.style(), ComponentTheme::Custom);
    QCOMPARE(theme.themeId(), QStringLiteral("dark"));
    QCOMPARE(theme.themeName(), QStringLiteral("Directory Dark"));
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#123456")));
    QCOMPARE(theme.buttonSize(), 50);
}

void ComponentThemeTest::availableThemesListsJsonFileIds()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(!writeThemeFile(dir.path(),
                            QStringLiteral("alpha.json"),
                            themeJson(QStringLiteral("alpha"),
                                      QStringLiteral("Alpha"),
                                      QStringLiteral("#101010"),
                                      40)).isEmpty());
    QVERIFY(!writeThemeFile(dir.path(),
                            QStringLiteral("beta.json"),
                            themeJson(QStringLiteral("beta"),
                                      QStringLiteral("Beta"),
                                      QStringLiteral("#202020"),
                                      41)).isEmpty());
    QVERIFY(!writeThemeFile(dir.path(), QStringLiteral("notes.txt"), QStringLiteral("ignored")).isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    theme.setThemeDirectory(dir.path());

    QCOMPARE(theme.availableThemes(), QStringList({ QStringLiteral("alpha"), QStringLiteral("beta") }));
}

void ComponentThemeTest::hotReloadUpdatesCurrentTheme()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeThemeFile(dir.path(),
                                        QStringLiteral("live.json"),
                                        themeJson(QStringLiteral("live"),
                                                  QStringLiteral("Live"),
                                                  QStringLiteral("#101010"),
                                                  40));
    QVERIFY(!path.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    theme.setHotReloadEnabled(true);
    QVERIFY(theme.loadThemeFile(path));
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#101010")));
    QTest::qWait(100);

    QSignalSpy spy(&theme, &ComponentTheme::styleChanged);
    QVERIFY(writeThemeFile(dir.path(),
                           QStringLiteral("live.json"),
                           themeJson(QStringLiteral("live"),
                                     QStringLiteral("Live"),
                                     QStringLiteral("#303030"),
                                     48)).size() > 0);

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#303030")));
    QCOMPARE(theme.buttonSize(), 48);
    QCOMPARE(theme.lastError(), QString());
}

void ComponentThemeTest::hotReloadSurvivesRepeatedAtomicRewrites()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeThemeFile(dir.path(),
                                        QStringLiteral("live.json"),
                                        themeJson(QStringLiteral("live"),
                                                  QStringLiteral("Live"),
                                                  QStringLiteral("#101010"),
                                                  40));
    QVERIFY(!path.isEmpty());

    ComponentTheme& theme = ComponentTheme::instance();
    theme.setHotReloadEnabled(true);
    QVERIFY(theme.loadThemeFile(path));
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#101010")));
    QTest::qWait(100);

    QSignalSpy spy(&theme, &ComponentTheme::styleChanged);
    QVERIFY(!atomicReplaceThemeFile(dir.path(),
                                    QStringLiteral("live.json"),
                                    themeJson(QStringLiteral("live"),
                                              QStringLiteral("Live"),
                                              QStringLiteral("#202020"),
                                              44)).isEmpty());
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#202020")));

    spy.clear();
    QVERIFY(!atomicReplaceThemeFile(dir.path(),
                                    QStringLiteral("live.json"),
                                    themeJson(QStringLiteral("live"),
                                              QStringLiteral("Live"),
                                              QStringLiteral("#303030"),
                                              48)).isEmpty());
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);
    QCOMPARE(theme.accent(), QColor(QStringLiteral("#303030")));
    QCOMPARE(theme.buttonSize(), 48);
}

QTEST_MAIN(ComponentThemeTest)

#include "tst_component_theme.moc"
