#include <QtTest/QtTest>

#include "TimelineViewport.h"

class TimelineViewportTest : public QObject
{
    Q_OBJECT

private slots:
    void fitRangeAddsMarginAndUpdatesCoordinateMapping();
    void zoomAtKeepsAnchorTimeStable();
    void shortRangesExpandToMinimumSpan();
    void viewWidthIsClampedToPositiveValue();
};

void TimelineViewportTest::fitRangeAddsMarginAndUpdatesCoordinateMapping()
{
    TimelineViewport viewport;
    viewport.setTotalStart(0);
    viewport.setTotalEnd(10000);
    viewport.setViewWidth(1000);

    viewport.fitRange(1000, 5000);

    QCOMPARE(viewport.viewStart(), 800);
    QCOMPARE(viewport.viewEnd(), 5200);
    QCOMPARE(viewport.viewSpan(), 4400);
    QCOMPARE(viewport.timeToPixel(800), 0.0);
    QCOMPARE(viewport.timeToPixel(5200), 1000.0);
    QCOMPARE(viewport.pixelToTime(500), 3000);
    QCOMPARE(viewport.durationToPixels(2200), 500.0);
    QCOMPARE(viewport.pixelsToDuration(250), 1100);
    QVERIFY(viewport.isVisible(3000));
    QVERIFY(viewport.isRangeVisible(700, 900));
    QVERIFY(!viewport.isRangeVisible(5200, 5300));
}

void TimelineViewportTest::zoomAtKeepsAnchorTimeStable()
{
    TimelineViewport viewport;
    viewport.setViewWidth(1000);
    viewport.fitRange(0, 10000);

    const qreal anchorPx = 250;
    const qint64 anchorTime = viewport.pixelToTime(anchorPx);
    const qint64 originalSpan = viewport.viewSpan();

    viewport.zoomAt(anchorPx, 2.0);

    QCOMPARE(viewport.viewSpan(), originalSpan / 2);
    QVERIFY(qAbs(viewport.pixelToTime(anchorPx) - anchorTime) <= 1);
}

void TimelineViewportTest::shortRangesExpandToMinimumSpan()
{
    TimelineViewport viewport;
    viewport.setTotalStart(0);
    viewport.setTotalEnd(10000);
    viewport.setViewWidth(1000);

    viewport.fitRange(4000, 4100);

    QCOMPARE(viewport.viewSpan(), TimelineViewport::kMinViewSpan);
    QCOMPARE(viewport.viewStart(), 3550);
    QCOMPARE(viewport.viewEnd(), 4550);
}

void TimelineViewportTest::viewWidthIsClampedToPositiveValue()
{
    TimelineViewport viewport;

    viewport.setViewWidth(0);

    QCOMPARE(viewport.viewWidth(), 1.0);
}

QTEST_APPLESS_MAIN(TimelineViewportTest)

#include "tst_timeline_viewport.moc"
