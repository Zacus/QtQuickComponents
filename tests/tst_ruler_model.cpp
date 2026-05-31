#include <QtTest/QtTest>

#include "RulerModel.h"
#include "TimelineViewport.h"

class RulerModelTest : public QObject
{
    Q_OBJECT

private slots:
    void setViewportGeneratesAlignedTicksAndRoles();
    void zoomedInViewportSelectsMillisecondLevel();
    void panningViewportRefreshesVisibleTicks();
    void clearingViewportResetsTicks();
};

void RulerModelTest::setViewportGeneratesAlignedTicksAndRoles()
{
    TimelineViewport viewport;
    viewport.setViewWidth(600);
    viewport.fitRange(60000, 660000);

    RulerModel model;
    model.setViewport(&viewport);

    QCOMPARE(model.majorInterval(), 300000);
    QCOMPARE(model.minorInterval(), 60000);
    QCOMPARE(model.majorFormat(), QStringLiteral("HH:mm"));
    QCOMPARE(model.rowCount(), 12);

    const RulerTick firstTick = model.tickAt(0);
    QCOMPARE(firstTick.timeMs(), 0);
    QVERIFY(firstTick.isMajor());
    QCOMPARE(firstTick.label(), QStringLiteral("00:00"));

    const QModelIndex firstIndex = model.index(0, 0);
    QCOMPARE(model.data(firstIndex, RulerModel::TimeMsRole).toLongLong(), firstTick.timeMs());
    QCOMPARE(model.data(firstIndex, RulerModel::IsMajorRole).toBool(), firstTick.isMajor());
    QCOMPARE(model.data(firstIndex, RulerModel::LabelRole).toString(), firstTick.label());

    const QHash<int, QByteArray> roles = model.roleNames();
    QCOMPARE(roles.value(RulerModel::TimeMsRole), QByteArray("timeMs"));
    QCOMPARE(roles.value(RulerModel::IsMajorRole), QByteArray("isMajor"));
    QCOMPARE(roles.value(RulerModel::LabelRole), QByteArray("label"));

    for (int i = 1; i < model.rowCount(); ++i)
        QCOMPARE(model.tickAt(i).timeMs() - model.tickAt(i - 1).timeMs(), model.minorInterval());
}

void RulerModelTest::zoomedInViewportSelectsMillisecondLevel()
{
    TimelineViewport viewport;
    viewport.setViewWidth(1000);
    viewport.fitRange(1000, 4000);

    RulerModel model;
    model.setViewport(&viewport);

    QCOMPARE(model.majorInterval(), 1000);
    QCOMPARE(model.minorInterval(), 200);
    QCOMPARE(model.majorFormat(), QStringLiteral("ss.zzz"));
    QVERIFY(model.rowCount() > 0);

    bool hasMajorTickAtOneSecond = false;
    for (int i = 0; i < model.rowCount(); ++i) {
        const RulerTick tick = model.tickAt(i);
        if (tick.timeMs() == 1000) {
            hasMajorTickAtOneSecond = tick.isMajor()
                && tick.label() == QStringLiteral("01.000");
            break;
        }
    }
    QVERIFY(hasMajorTickAtOneSecond);
}

void RulerModelTest::panningViewportRefreshesVisibleTicks()
{
    TimelineViewport viewport;
    viewport.setViewWidth(600);
    viewport.fitRange(60000, 660000);

    RulerModel model;
    model.setViewport(&viewport);

    QSignalSpy ticksChangedSpy(&model, &RulerModel::ticksChanged);
    const qint64 firstBefore = model.tickAt(0).timeMs();
    const int countBefore = model.rowCount();

    viewport.panBy(60);

    QVERIFY(ticksChangedSpy.count() > 0);
    QVERIFY(model.tickAt(0).timeMs() > firstBefore);
    QVERIFY(model.rowCount() <= countBefore);
    for (int i = 1; i < model.rowCount(); ++i)
        QCOMPARE(model.tickAt(i).timeMs() - model.tickAt(i - 1).timeMs(), model.minorInterval());
}

void RulerModelTest::clearingViewportResetsTicks()
{
    TimelineViewport viewport;
    viewport.setViewWidth(600);
    viewport.fitRange(60000, 660000);

    RulerModel model;
    model.setViewport(&viewport);
    QVERIFY(model.rowCount() > 0);

    model.setViewport(nullptr);

    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.majorInterval(), 300000);
    QCOMPARE(model.minorInterval(), 60000);
}

QTEST_APPLESS_MAIN(RulerModelTest)

#include "tst_ruler_model.moc"
