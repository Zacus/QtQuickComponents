#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "ThemeFileWatcher.h"

class ThemeFileWatcherTest : public QObject
{
    Q_OBJECT

private slots:
    void emitsReloadRequestedForFileRewrite();
    void emitsReloadRequestedForAtomicReplace();

private:
    static QString writeFile(const QString& directory, const QString& fileName, const QString& text)
    {
        const QString path = QDir(directory).filePath(fileName);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return {};
        file.write(text.toUtf8());
        return path;
    }

    static QString atomicReplace(const QString& directory, const QString& fileName, const QString& text)
    {
        const QString path = QDir(directory).filePath(fileName);
        const QString tempPath = QDir(directory).filePath(fileName + QStringLiteral(".tmp"));
        QFile temp(tempPath);
        if (!temp.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return {};
        temp.write(text.toUtf8());
        temp.close();
        QFile::remove(path);
        if (!QFile::rename(tempPath, path))
            return {};
        return path;
    }
};

void ThemeFileWatcherTest::emitsReloadRequestedForFileRewrite()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeFile(dir.path(), QStringLiteral("theme.json"), QStringLiteral("{}"));
    QVERIFY(!path.isEmpty());

    ThemeFileWatcher watcher;
    watcher.setEnabled(true);
    watcher.setWatchedFile(path);
    QSignalSpy spy(&watcher, &ThemeFileWatcher::reloadRequested);

    QVERIFY(!writeFile(dir.path(), QStringLiteral("theme.json"), QStringLiteral("{\"a\":1}")).isEmpty());

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);
}

void ThemeFileWatcherTest::emitsReloadRequestedForAtomicReplace()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = writeFile(dir.path(), QStringLiteral("theme.json"), QStringLiteral("{}"));
    QVERIFY(!path.isEmpty());

    ThemeFileWatcher watcher;
    watcher.setEnabled(true);
    watcher.setWatchedFile(path);
    QSignalSpy spy(&watcher, &ThemeFileWatcher::reloadRequested);

    QVERIFY(!atomicReplace(dir.path(), QStringLiteral("theme.json"), QStringLiteral("{\"a\":2}")).isEmpty());

    QTRY_VERIFY_WITH_TIMEOUT(spy.count() > 0, 2000);
}

QTEST_MAIN(ThemeFileWatcherTest)

#include "tst_theme_file_watcher.moc"
