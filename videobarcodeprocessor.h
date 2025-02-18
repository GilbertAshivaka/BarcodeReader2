#ifndef VIDEOBARCODEPROCESSOR_H
#define VIDEOBARCODEPROCESSOR_H

#include <QObject>
#include <QVideoFrame>
#include <QVideoSink>
#include "include/DynamsoftCaptureVisionRouter.h"
#include "include/DynamsoftUtility.h"

// Forward declarations from the SDK
using namespace dynamsoft::cvr;
using namespace dynamsoft::dbr;
using namespace dynamsoft::utility;
using namespace dynamsoft::license;

// --- CapturedResultReceiver Declaration ---
// This class is now declared at global scope so that Q_OBJECT works.
class CapturedResultReceiver : public QObject, public CCapturedResultReceiver {
    Q_OBJECT
public:
    explicit CapturedResultReceiver(QObject* parent = nullptr) : QObject(parent) {}
    virtual void OnDecodedBarcodesReceived(CDecodedBarcodesResult* pResult) override;
signals:
    void resultReceived(const QString &result);
};

// --- VideoBarcodeProcessor Declaration ---
class VideoBarcodeProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVideoSink* videoSink READ videoSink WRITE setVideoSink NOTIFY videoSinkChanged)
public:
    explicit VideoBarcodeProcessor(QObject *parent = nullptr);
    ~VideoBarcodeProcessor();

    QVideoSink* videoSink() const;
    void setVideoSink(QVideoSink* sink);

public slots:
    void processFrame(const QVideoFrame &frame);

signals:
    void barcodeResult(const QString &result);
    void videoSinkChanged();

private:
    // The router that handles the asynchronous pipeline.
    CCaptureVisionRouter* m_cvr = nullptr;

    // Custom video fetcher to feed images into the router.
    class MyVideoFetcher : public CImageSourceAdapter {
    public:
        MyVideoFetcher() {}
        ~MyVideoFetcher() {}
        bool HasNextImageToFetch() const override { return true; }
        void MyAddImageToBuffer(const CImageData *img, bool bClone = true) {
            AddImageToBuffer(img, bClone);
        }
    };
    MyVideoFetcher* m_fetcher = nullptr;

    // Custom result receiver (now declared externally)
    CapturedResultReceiver* m_receiver = nullptr;

    CMultiFrameResultCrossFilter* m_filter = nullptr;

    QVideoSink* m_videoSink = nullptr;
    quint64 m_frameId = 0;
};

#endif // VIDEOBARCODEPROCESSOR_H
