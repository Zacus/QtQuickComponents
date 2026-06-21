#include "GlobalVideoRenderer.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QImage>
#include <QString>
#include <QTextStream>

#include <algorithm>
#include <atomic>
#include <ctime>
#include <limits>
#include <thread>
#include <vector>

using QuickUI::Components::Internal::GlobalVideoRenderer;

namespace {

enum class FrameMode {
    Rgba,
    Yuv420
};

struct Options
{
    int channels = 64;
    int framesPerChannel = 120;
    int threads = std::max(1u, std::thread::hardware_concurrency());
    int width = 320;
    int height = 180;
    int warmupFramesPerChannel = 4;
    FrameMode mode = FrameMode::Yuv420;
};

struct WorkerStats
{
    quint64 pushes = 0;
    quint64 failures = 0;
    qint64 totalPushNs = 0;
    qint64 maxPushNs = 0;
};

int parsePositiveInt(const QCommandLineParser& parser,
                     const QString& name,
                     int fallback,
                     QTextStream& err)
{
    const QString value = parser.value(name);
    if (value.isEmpty())
        return fallback;

    bool ok = false;
    const int parsed = value.toInt(&ok);
    if (!ok || parsed <= 0) {
        err << "Invalid --" << name << ": expected a positive integer, got " << value << "\n";
        return -1;
    }
    return parsed;
}

QString modeName(FrameMode mode)
{
    return mode == FrameMode::Yuv420 ? QStringLiteral("yuv420") : QStringLiteral("rgba");
}

GlobalVideoRenderer::Yuv420Frame makeYuv420Frame(int width, int height)
{
    GlobalVideoRenderer::Yuv420Frame frame;
    frame.width = width;
    frame.height = height;
    frame.yStride = width;
    frame.uStride = width / 2;
    frame.vStride = width / 2;
    frame.yPlane = QByteArray(width * height, char(96));
    frame.uPlane = QByteArray((width / 2) * (height / 2), char(128));
    frame.vPlane = QByteArray((width / 2) * (height / 2), char(128));
    return frame;
}

QImage makeRgbaFrame(int width, int height)
{
    QImage image(width, height, QImage::Format_RGBA8888);
    image.fill(QColor(18, 52, 86));
    return image;
}

bool pushFrame(GlobalVideoRenderer& renderer,
               FrameMode mode,
               int channelId,
               const QImage& rgbaFrame,
               const GlobalVideoRenderer::Yuv420Frame& yuvFrame)
{
    if (mode == FrameMode::Yuv420)
        return renderer.pushYuv420Frame(channelId, yuvFrame);
    return renderer.pushFrame(channelId, rgbaFrame);
}

Options parseOptions(const QCoreApplication& app, bool& ok)
{
    QTextStream err(stderr);
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Manual GlobalVideoRenderer stress baseline. Not registered in CTest."));
    parser.addHelpOption();
    parser.addOption({QStringLiteral("channels"),
                      QStringLiteral("Number of channels to rotate through."),
                      QStringLiteral("count"),
                      QStringLiteral("64")});
    parser.addOption({QStringLiteral("frames"),
                      QStringLiteral("Measured frames pushed per channel."),
                      QStringLiteral("count"),
                      QStringLiteral("120")});
    parser.addOption({QStringLiteral("threads"),
                      QStringLiteral("Worker thread count."),
                      QStringLiteral("count"),
                      QString::number(std::max(1u, std::thread::hardware_concurrency()))});
    parser.addOption({QStringLiteral("width"),
                      QStringLiteral("Frame width. YUV420 mode requires an even value."),
                      QStringLiteral("pixels"),
                      QStringLiteral("320")});
    parser.addOption({QStringLiteral("height"),
                      QStringLiteral("Frame height. YUV420 mode requires an even value."),
                      QStringLiteral("pixels"),
                      QStringLiteral("180")});
    parser.addOption({QStringLiteral("warmup"),
                      QStringLiteral("Warmup frames pushed per channel before measurement."),
                      QStringLiteral("count"),
                      QStringLiteral("4")});
    parser.addOption({QStringLiteral("format"),
                      QStringLiteral("Frame format: yuv420 or rgba."),
                      QStringLiteral("name"),
                      QStringLiteral("yuv420")});

    parser.process(app);

    Options options;
    options.channels = parsePositiveInt(parser, QStringLiteral("channels"), options.channels, err);
    options.framesPerChannel = parsePositiveInt(parser, QStringLiteral("frames"), options.framesPerChannel, err);
    options.threads = parsePositiveInt(parser, QStringLiteral("threads"), options.threads, err);
    options.width = parsePositiveInt(parser, QStringLiteral("width"), options.width, err);
    options.height = parsePositiveInt(parser, QStringLiteral("height"), options.height, err);
    options.warmupFramesPerChannel = parsePositiveInt(parser, QStringLiteral("warmup"), options.warmupFramesPerChannel, err);

    const QString format = parser.value(QStringLiteral("format")).toLower();
    if (format == QStringLiteral("yuv420")) {
        options.mode = FrameMode::Yuv420;
    } else if (format == QStringLiteral("rgba")) {
        options.mode = FrameMode::Rgba;
    } else {
        err << "Invalid --format: expected yuv420 or rgba, got " << format << "\n";
        ok = false;
        return options;
    }

    ok = options.channels > 0
        && options.framesPerChannel > 0
        && options.threads > 0
        && options.width > 0
        && options.height > 0
        && options.warmupFramesPerChannel > 0;

    if (ok && options.mode == FrameMode::Yuv420
        && ((options.width % 2) != 0 || (options.height % 2) != 0)) {
        err << "YUV420 mode requires even width and height\n";
        ok = false;
    }

    return options;
}

void runPushes(GlobalVideoRenderer& renderer,
               const Options& options,
               quint64 pushes,
               std::vector<WorkerStats>* workerStats)
{
    std::atomic<quint64> nextPush = 0;
    std::vector<std::thread> workers;
    workers.reserve(static_cast<size_t>(options.threads));

    for (int workerIndex = 0; workerIndex < options.threads; ++workerIndex) {
        workers.emplace_back([&, workerIndex]() {
            const QImage rgbaFrame = makeRgbaFrame(options.width, options.height);
            const GlobalVideoRenderer::Yuv420Frame yuvFrame = makeYuv420Frame(options.width, options.height);
            WorkerStats localStats;

            while (true) {
                const quint64 pushIndex = nextPush.fetch_add(1, std::memory_order_relaxed);
                if (pushIndex >= pushes)
                    break;

                const int channelId = static_cast<int>(pushIndex % quint64(options.channels));
                QElapsedTimer pushTimer;
                pushTimer.start();
                const bool accepted = pushFrame(renderer, options.mode, channelId, rgbaFrame, yuvFrame);
                const qint64 elapsedNs = pushTimer.nsecsElapsed();

                ++localStats.pushes;
                localStats.totalPushNs += elapsedNs;
                localStats.maxPushNs = std::max(localStats.maxPushNs, elapsedNs);
                if (!accepted)
                    ++localStats.failures;
            }

            if (workerStats)
                workerStats->at(static_cast<size_t>(workerIndex)) = localStats;
        });
    }

    for (std::thread& worker : workers)
        worker.join();
}

} // namespace

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("manual_video_stress"));

    bool optionsOk = false;
    const Options options = parseOptions(app, optionsOk);
    if (!optionsOk)
        return 2;

    GlobalVideoRenderer renderer;
    const quint64 measuredPushes = quint64(options.channels) * quint64(options.framesPerChannel);
    const quint64 warmupPushes = quint64(options.channels) * quint64(options.warmupFramesPerChannel);

    runPushes(renderer, options, warmupPushes, nullptr);

    std::vector<WorkerStats> workerStats(static_cast<size_t>(options.threads));
    const std::clock_t cpuStart = std::clock();
    QElapsedTimer wallTimer;
    wallTimer.start();
    runPushes(renderer, options, measuredPushes, &workerStats);
    const qint64 wallNs = wallTimer.nsecsElapsed();
    const std::clock_t cpuEnd = std::clock();

    quint64 acceptedPushes = 0;
    quint64 failures = 0;
    qint64 totalPushNs = 0;
    qint64 maxPushNs = 0;
    for (const WorkerStats& stats : workerStats) {
        acceptedPushes += stats.pushes;
        failures += stats.failures;
        totalPushNs += stats.totalPushNs;
        maxPushNs = std::max(maxPushNs, stats.maxPushNs);
    }

    quint64 minSerial = std::numeric_limits<quint64>::max();
    quint64 maxSerial = 0;
    int channelsWithFrames = 0;
    for (int channelId = 0; channelId < options.channels; ++channelId) {
        const quint64 serial = renderer.frameSerial(channelId);
        if (serial == 0)
            continue;
        minSerial = std::min(minSerial, serial);
        maxSerial = std::max(maxSerial, serial);
        ++channelsWithFrames;
    }

    const double wallMs = double(wallNs) / 1'000'000.0;
    const double wallSeconds = double(wallNs) / 1'000'000'000.0;
    const double cpuMs = 1000.0 * double(cpuEnd - cpuStart) / double(CLOCKS_PER_SEC);
    const double avgPushUs = acceptedPushes > 0
        ? double(totalPushNs) / double(acceptedPushes) / 1000.0
        : 0.0;
    const double maxPushUs = double(maxPushNs) / 1000.0;
    const double aggregateFps = wallSeconds > 0.0 ? double(acceptedPushes) / wallSeconds : 0.0;
    const double singleCoreCpuPercent = wallMs > 0.0 ? (cpuMs / wallMs) * 100.0 : 0.0;

    QTextStream out(stdout);
    out.setRealNumberPrecision(3);
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out << "manual_video_stress\n";
    out << "format: " << modeName(options.mode) << "\n";
    out << "channels: " << options.channels << "\n";
    out << "framesPerChannel: " << options.framesPerChannel << "\n";
    out << "threads: " << options.threads << "\n";
    out << "resolution: " << options.width << "x" << options.height << "\n";
    out << "warmupPushes: " << warmupPushes << "\n";
    out << "measuredPushes: " << acceptedPushes << "\n";
    out << "failures: " << failures << "\n";
    out << "wallMs: " << wallMs << "\n";
    out << "processCpuMs: " << cpuMs << "\n";
    out << "singleCoreCpuPercent: " << singleCoreCpuPercent << "\n";
    out << "aggregateFps: " << aggregateFps << "\n";
    out << "avgPushUs: " << avgPushUs << "\n";
    out << "maxPushUs: " << maxPushUs << "\n";
    out << "channelsWithFrames: " << channelsWithFrames << "\n";
    out << "latestSerialMin: " << (channelsWithFrames > 0 ? minSerial : 0) << "\n";
    out << "latestSerialMax: " << maxSerial << "\n";

    return failures == 0 && acceptedPushes == measuredPushes ? 0 : 1;
}
