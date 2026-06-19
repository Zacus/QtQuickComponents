#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QPointer>
#include <QString>
#include <QVariantList>

class WndViewModel : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int wndId READ wndId WRITE setWndId NOTIFY wndIdChanged)
    Q_PROPERTY(int channelId READ channelId WRITE setChannelId NOTIFY channelIdChanged)
    Q_PROPERTY(QString channelName READ channelName WRITE setChannelName NOTIFY channelNameChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool isRecording READ isRecording WRITE setRecording NOTIFY recordingChanged)
    Q_PROPERTY(AlarmLevel alarmLevel READ alarmLevel WRITE setAlarmLevel NOTIFY alarmChanged)
    Q_PROPERTY(SignalState signalState READ signalState WRITE setSignalState NOTIFY signalStateChanged)
    Q_PROPERTY(QVariantList osdModel READ osdModel WRITE setOsdModel NOTIFY osdChanged)
    Q_PROPERTY(QObject* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)

public:
    enum AlarmLevel {
        None,
        Warning,
        Critical
    };
    Q_ENUM(AlarmLevel)

    enum SignalState {
        Normal,
        NoSignal,
        Connecting
    };
    Q_ENUM(SignalState)

    explicit WndViewModel(QObject* parent = nullptr);

    int wndId() const { return m_wndId; }
    int channelId() const { return m_channelId; }
    QString channelName() const { return m_channelName; }
    bool isActive() const { return m_isActive; }
    bool isRecording() const { return m_isRecording; }
    AlarmLevel alarmLevel() const { return m_alarmLevel; }
    SignalState signalState() const { return m_signalState; }
    QVariantList osdModel() const { return m_osdModel; }
    QObject* videoSink() const { return m_videoSink.data(); }

    void setWndId(int wndId);
    void setChannelId(int channelId);
    void setChannelName(const QString& channelName);
    void setActive(bool active);
    void setRecording(bool recording);
    void setAlarmLevel(AlarmLevel alarmLevel);
    void setSignalState(SignalState signalState);
    void setOsdModel(const QVariantList& osdModel);
    void setVideoSink(QObject* videoSink);

signals:
    void wndIdChanged();
    void channelIdChanged();
    void channelNameChanged();
    void activeChanged();
    void recordingChanged();
    void alarmChanged();
    void signalStateChanged();
    void osdChanged();
    void videoSinkChanged();

    void clicked(int wndId);
    void doubleClicked(int wndId, int channelId);
    void screenshotRequested(int wndId, int channelId);
    void recordToggled(int wndId, int channelId);
    void channelClosed(int wndId, int channelId);
    void dragStarted(int wndId, int channelId);
    void dropReceived(int wndId, int fromChannelId);

private:
    int m_wndId = 0;
    int m_channelId = -1;
    QString m_channelName;
    bool m_isActive = false;
    bool m_isRecording = false;
    AlarmLevel m_alarmLevel = None;
    SignalState m_signalState = NoSignal;
    QVariantList m_osdModel;
    QPointer<QObject> m_videoSink;
};
