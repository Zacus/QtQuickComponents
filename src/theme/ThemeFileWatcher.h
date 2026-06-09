#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>

class ThemeFileWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ThemeFileWatcher(QObject* parent = nullptr);

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    void setWatchedFile(const QString& path);
    QString watchedFile() const { return m_watchedFile; }

    void clear();

signals:
    void reloadRequested();

private:
    void configure();
    void clearWatchPaths();
    void scheduleReload();
    void pollFileTimestamp();

    bool m_enabled = false;
    QString m_watchedFile;
    QFileSystemWatcher m_watcher;
    QTimer m_debounceTimer;
    QTimer m_pollTimer;
    qint64 m_lastModifiedMs = 0;
    qint64 m_lastSize = -1;
};
