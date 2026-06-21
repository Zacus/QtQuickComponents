#include <QtTest/QtTest>

#include <type_traits>

#include "WndViewModel.h"

static_assert(std::is_same_v<WndViewModel, QuickUI::Components::WndViewModel>);

class WndViewModelTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultsMatchEmptyWindow();
    void propertySettersEmitOnlyOnChange();
};

void WndViewModelTest::defaultsMatchEmptyWindow()
{
    WndViewModel vm;

    QCOMPARE(vm.wndId(), 0);
    QCOMPARE(vm.channelId(), -1);
    QCOMPARE(vm.channelName(), QString());
    QCOMPARE(vm.isActive(), false);
    QCOMPARE(vm.isRecording(), false);
    QCOMPARE(vm.alarmLevel(), WndViewModel::None);
    QCOMPARE(vm.signalState(), WndViewModel::NoSignal);
    QCOMPARE(vm.osdModel(), QVariantList());
    QCOMPARE(vm.videoSink(), nullptr);
}

void WndViewModelTest::propertySettersEmitOnlyOnChange()
{
    WndViewModel vm;
    QSignalSpy channelSpy(&vm, &WndViewModel::channelIdChanged);
    QSignalSpy signalSpy(&vm, &WndViewModel::signalStateChanged);

    vm.setChannelId(7);
    vm.setChannelId(7);
    vm.setSignalState(WndViewModel::Normal);
    vm.setSignalState(WndViewModel::Normal);

    QCOMPARE(vm.channelId(), 7);
    QCOMPARE(channelSpy.count(), 1);
    QCOMPARE(vm.signalState(), WndViewModel::Normal);
    QCOMPARE(signalSpy.count(), 1);
}

QTEST_APPLESS_MAIN(WndViewModelTest)

#include "tst_wnd_view_model.moc"
