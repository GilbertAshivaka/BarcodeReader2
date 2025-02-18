#include <QGuiApplication>
#include <QQmlApplicationEngine>

#include "videobarcodeprocessor.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<VideoBarcodeProcessor>("com.videobarcode", 1, 0, "VideoBarcodeProcessor");

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/BarcodeReader2/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
