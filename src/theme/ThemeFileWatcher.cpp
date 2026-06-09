#include "ThemeFileWatcher.h"

#include <QDateTime>
#include <QFileInfo>
#include <QStringList>

ThemeFileWatcher::ThemeFileWatcher(QObject* parent)
    : QObject(parent)
{
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(80);
    m_pollTimer.setInterval(100);

    connect(&m_watcher, &QFileSystemWatcher::fileChanged,
            this, [this](const QString&) { scheduleReload(); });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, [this](const QString&) { scheduleReload(); });
    connect(&m_debounceTimer, &QTimer::timeout,
            this, [this]() { emit reloadRequested(); });
    connect(&m_pollTimer, &QTimer::timeout,
            this, [this]() { pollFileTimestamp(); });
}

void ThemeFileWatcher::setEnabled(bool enabled)
{
    if (m_enabled == enabled) return;

    m_enabled = enabled;
    configure();
}

void ThemeFileWatcher::setWatchedFile(const QString& path)
{
    const QString normalized = path.isEmpty()
        ? QString()
        : QFileInfo(path).absoluteFilePath();
    if (m_watchedFile == normalized) {
        if (m_enabled && m_watcher.files().isEmpty() && m_watcher.directories().isEmpty()) {
            configure();
        }
        return;
    }

    m_watchedFile = normalized;
    configure();
}

void ThemeFileWatcher::clear()
{
    m_watchedFile.clear();
    clearWatchPaths();
}

void ThemeFileWatcher::configure()
{
    clearWatchPaths();
    if (!m_enabled || m_watchedFile.isEmpty()) {
        return;
    }

    const QFileInfo info(m_watchedFile);
    if (!info.exists()) {
        return;
    }

    m_lastModifiedMs = info.lastModified().toMSecsSinceEpoch();
    m_lastSize = info.size();
    m_watcher.addPath(info.absoluteFilePath());
    m_watcher.addPath(info.absolutePath());
    m_pollTimer.start();
}

void ThemeFileWatcher::clearWatchPaths()
{
    m_debounceTimer.stop();
    const QStringList files = m_watcher.files();
    if (!files.isEmpty()) {
        m_watcher.removePaths(files);
    }
    const QStringList directories = m_watcher.directories();
    if (!directories.isEmpty()) {
        m_watcher.removePaths(directories);
    }
    m_pollTimer.stop();
    m_lastModifiedMs = 0;
    m_lastSize = -1;
}

void ThemeFileWatcher::scheduleReload()
{
    if (!m_enabled) {
        return;
    }

    const QFileInfo info(m_watchedFile);
    if (info.exists()) {
        m_lastModifiedMs = info.lastModified().toMSecsSinceEpoch();
        m_lastSize = info.size();
    }
    m_debounceTimer.start();
}

void ThemeFileWatcher::pollFileTimestamp()
{
    if (m_watchedFile.isEmpty()) {
        return;
    }

    const QFileInfo info(m_watchedFile);
    if (!info.exists()) {
        return;
    }

    const qint64 lastModifiedMs = info.lastModified().toMSecsSinceEpoch();
    const qint64 size = info.size();
    if (m_lastModifiedMs == 0 || m_lastSize < 0) {
        m_lastModifiedMs = lastModifiedMs;
        m_lastSize = size;
        return;
    }
    if (lastModifiedMs != m_lastModifiedMs || size != m_lastSize) {
        m_lastModifiedMs = lastModifiedMs;
        m_lastSize = size;
        scheduleReload();
    }
}
