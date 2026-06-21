#include "GlobalVideoRenderer.h"

#include <QColor>
#include <QImage>
#include <QObject>
#include <QQmlEngine>
#include <QtQuickTest/quicktest.h>

using QuickUI::Components::Internal::GlobalVideoRenderer;

class TestVideoFrameSource : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    Q_INVOKABLE bool pushSolidFrame(QObject* videoSink, int channelId)
    {
        auto* renderer = qobject_cast<GlobalVideoRenderer*>(videoSink);
        if (!renderer)
            return false;

        QImage image(16, 16, QImage::Format_RGBA8888);
        image.fill(QColor(18, 52, 86));
        return renderer->pushFrame(channelId, image);
    }
};

int main(int argc, char** argv)
{
    qmlRegisterType<TestVideoFrameSource>("QuickUI.Components.Tests", 1, 0, "TestVideoFrameSource");
    return quick_test_main(argc, argv, "qml_components", nullptr);
}

#include "tst_qml_components.moc"
