#include <QtTest/QtTest>

#include "TimelineEnums.h"
#include "TimelineModel.h"

class TimelineModelTest : public QObject
{
    Q_OBJECT

private slots:
    void addSegmentKeepsRowsSortedAndUpdatesBounds();
    void mergeOverlappingOnlyMergesMatchingTypes();
    void segmentsInRangeIncludesIntervalsThatStartBeforeView();
    void segmentIndexAtFindsEarlierOverlappingInterval();
};

void TimelineModelTest::addSegmentKeepsRowsSortedAndUpdatesBounds()
{
    TimelineModel model;

    model.addSegment(300, 400, TimelineEnums::SegmentAlarm);
    model.addSegment(100, 200, TimelineEnums::SegmentNormal);
    model.addSegment(220, 260, TimelineEnums::SegmentMotion);

    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.totalStart(), 100);
    QCOMPARE(model.totalEnd(), 400);
    QCOMPARE(model.data(model.index(0, 0), TimelineModel::StartMsRole).toLongLong(), 100);
    QCOMPARE(model.data(model.index(1, 0), TimelineModel::StartMsRole).toLongLong(), 220);
    QCOMPARE(model.data(model.index(2, 0), TimelineModel::StartMsRole).toLongLong(), 300);
}

void TimelineModelTest::mergeOverlappingOnlyMergesMatchingTypes()
{
    TimelineModel model;

    model.addSegment(0, 100, TimelineEnums::SegmentNormal);
    model.addSegment(100, 180, TimelineEnums::SegmentNormal);
    model.addSegment(170, 240, TimelineEnums::SegmentMotion);

    model.mergeOverlapping();

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.segmentAt(0).startMs(), 0);
    QCOMPARE(model.segmentAt(0).endMs(), 180);
    QCOMPARE(model.segmentAt(0).type(), int(TimelineEnums::SegmentNormal));
    QCOMPARE(model.segmentAt(1).startMs(), 170);
    QCOMPARE(model.segmentAt(1).endMs(), 240);
    QCOMPARE(model.segmentAt(1).type(), int(TimelineEnums::SegmentMotion));
}

void TimelineModelTest::segmentsInRangeIncludesIntervalsThatStartBeforeView()
{
    TimelineModel model;

    model.addSegment(0, 1000, TimelineEnums::SegmentNormal);
    model.addSegment(1200, 1400, TimelineEnums::SegmentAlarm);

    const QList<TimelineSegment> result = model.segmentsInRawRange(500, 700);

    QCOMPARE(result.size(), 1);
    QCOMPARE(result.first().startMs(), 0);
    QCOMPARE(result.first().endMs(), 1000);
}

void TimelineModelTest::segmentIndexAtFindsEarlierOverlappingInterval()
{
    TimelineModel model;

    model.addSegment(0, 1000, TimelineEnums::SegmentNormal);
    model.addSegment(100, 200, TimelineEnums::SegmentMotion);

    QCOMPARE(model.segmentIndexAt(500), 0);
}

QTEST_APPLESS_MAIN(TimelineModelTest)

#include "tst_timeline_model.moc"
