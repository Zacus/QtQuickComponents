#include "WndViewModel.h"

WndViewModel::WndViewModel(QObject* parent)
    : QObject(parent)
{
}

void WndViewModel::setWndId(int wndId)
{
    if (m_wndId == wndId)
        return;

    m_wndId = wndId;
    emit wndIdChanged();
}

void WndViewModel::setChannelId(int channelId)
{
    if (m_channelId == channelId)
        return;

    m_channelId = channelId;
    emit channelIdChanged();
}

void WndViewModel::setChannelName(const QString& channelName)
{
    if (m_channelName == channelName)
        return;

    m_channelName = channelName;
    emit channelNameChanged();
}

void WndViewModel::setActive(bool active)
{
    if (m_isActive == active)
        return;

    m_isActive = active;
    emit activeChanged();
}

void WndViewModel::setRecording(bool recording)
{
    if (m_isRecording == recording)
        return;

    m_isRecording = recording;
    emit recordingChanged();
}

void WndViewModel::setAlarmLevel(WndViewModel::AlarmLevel alarmLevel)
{
    if (m_alarmLevel == alarmLevel)
        return;

    m_alarmLevel = alarmLevel;
    emit alarmChanged();
}

void WndViewModel::setSignalState(WndViewModel::SignalState signalState)
{
    if (m_signalState == signalState)
        return;

    m_signalState = signalState;
    emit signalStateChanged();
}

void WndViewModel::setOsdModel(const QVariantList& osdModel)
{
    if (m_osdModel == osdModel)
        return;

    m_osdModel = osdModel;
    emit osdChanged();
}

void WndViewModel::setVideoSink(QObject* videoSink)
{
    if (m_videoSink == videoSink)
        return;

    m_videoSink = videoSink;
    emit videoSinkChanged();
}
