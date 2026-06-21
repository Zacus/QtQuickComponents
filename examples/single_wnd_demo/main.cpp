#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTimer>

#if defined(QT_STATIC)
#  include <QtPlugin>
Q_IMPORT_QML_PLUGIN(QuickUI_ComponentsPlugin)
Q_IMPORT_QML_PLUGIN(QuickUI_Components_implPlugin)
#endif

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption quitAfterOption(
        QStringLiteral("quit-after-ms"),
        QStringLiteral("Quit automatically after the given number of milliseconds."),
        QStringLiteral("milliseconds"));
    parser.addOption(quitAfterOption);
    parser.process(app);

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral(QTC_DEMO_QML_IMPORT_PATH));

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        [] { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("SingleWndDemo", "Main");

    bool ok = false;
    const int quitAfterMs = parser.value(quitAfterOption).toInt(&ok);
    if (ok && quitAfterMs >= 0)
        QTimer::singleShot(quitAfterMs, &app, &QCoreApplication::quit);

    return app.exec();
}
