#include "videobarcodeprocessor.h"
#include <QImage>
#include <QDebug>
#include <chrono>
#include "include/DynamsoftBarcodeReader.h" // For pixel format constants

using namespace std::chrono;

// --- Implementation of CapturedResultReceiver ---
void CapturedResultReceiver::OnDecodedBarcodesReceived(CDecodedBarcodesResult* pResult)
{
    QString resultString;
    if (pResult->GetErrorCode() != EC_OK) {
        resultString = QString("Error: %1").arg(pResult->GetErrorString());
    } else {
        auto tag = pResult->GetOriginalImageTag();
        if (tag) {
            resultString += QString("ImageID: %1\n").arg(tag->GetImageId());
        }
        int count = pResult->GetItemsCount();
        resultString += QString("Decoded %1 barcodes\n").arg(count);
        for (int i = 0; i < count; i++) {
            const CBarcodeResultItem* item = pResult->GetItem(i);
            if (item) {
                resultString += QString("Result %1\nBarcode Format: %2\nBarcode Text: %3\n--------------------\n")
                                    .arg(i + 1)
                                    .arg(item->GetFormatString())
                                    .arg(item->GetText());
            }
        }
    }
    emit resultReceived(resultString);
}

// --- Implementation of VideoBarcodeProcessor ---
VideoBarcodeProcessor::VideoBarcodeProcessor(QObject *parent)
    : QObject(parent)
{
    // Initialize license (adjust your license string as needed)
    char errorMsg[256];
    int ret = CLicenseManager::InitLicense("t0068lQAAABa8p7d6vIsLzXULX4XjMq689SBDTTjrc+ysij5GMVzyfNu0DF7yMUPWY+FmrDMolnhZSBqdHn9nzwVElDQ/8aY=;t0068lQAAAGXVdR0NdbdrG3GmOSmy5GOkKEP4v8XG+qCXVenT2I9CHYIDns4ZHJc0FE+BxPXzC1tbsOMkKlWxRCbz3F7dCqE=", errorMsg, 256);
    qDebug() << "License init:" << errorMsg;

    // Create the Capture Vision Router
    m_cvr = new CCaptureVisionRouter;

    // Create and configure the video fetcher.
    m_fetcher = new MyVideoFetcher;
    m_fetcher->SetMaxImageCount(100);
    m_fetcher->SetBufferOverflowProtectionMode(BOPM_UPDATE);
    m_fetcher->SetColourChannelUsageType(CCUT_AUTO);
    m_cvr->SetInput(m_fetcher);

    // Create and add a multi-frame result cross filter.
    m_filter = new CMultiFrameResultCrossFilter;
    m_filter->EnableResultCrossVerification(CRIT_BARCODE, true);
    m_filter->EnableResultDeduplication(CRIT_BARCODE, true);
    m_filter->SetDuplicateForgetTime(CRIT_BARCODE, 5000);
    m_cvr->AddResultFilter(m_filter);

    // Create and add our result receiver.
    m_receiver = new CapturedResultReceiver;
    connect(m_receiver, &CapturedResultReceiver::resultReceived, this, &VideoBarcodeProcessor::barcodeResult);
    m_cvr->AddResultReceiver(m_receiver);

    // Start capturing using a preset template for reading barcodes.
    char errorMsgBuffer[512] = {0};
    int errorCode = m_cvr->StartCapturing(CPresetTemplate::PT_READ_BARCODES, false, errorMsgBuffer, 512);
    if (errorCode != EC_OK) {
        qDebug() << "Error starting capture:" << errorMsgBuffer;
    }
}

VideoBarcodeProcessor::~VideoBarcodeProcessor()
{
    if (m_cvr) {
        m_cvr->StopCapturing(false, true);
        delete m_cvr;
        m_cvr = nullptr;
    }
    // Optionally, clean up other allocated objects if not managed by the router.
}

QVideoSink* VideoBarcodeProcessor::videoSink() const
{
    return m_videoSink;
}

void VideoBarcodeProcessor::setVideoSink(QVideoSink* sink)
{
    if (m_videoSink) {
        disconnect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoBarcodeProcessor::processFrame);
    }
    m_videoSink = sink;
    if (m_videoSink) {
        connect(m_videoSink, &QVideoSink::videoFrameChanged, this, &VideoBarcodeProcessor::processFrame);
    }
    emit videoSinkChanged();
}

void VideoBarcodeProcessor::processFrame(const QVideoFrame &frame)
{
    if (!frame.isValid())
        return;

    // Convert the QVideoFrame to a QImage.
    QImage image = frame.toImage();
    if (image.isNull())
        return;

    // Convert the image to RGB888 (the SDK expects a standard pixel layout).
    QImage rgbImage = image.convertToFormat(QImage::Format_RGB888);

    // Create an image tag to track the frame.
    CFileImageTag tag(nullptr, 0, 0);
    tag.SetImageId(++m_frameId);

    // Create CImageData using the QImage data.
    int dataSize = rgbImage.height() * rgbImage.bytesPerLine();
    CImageData data(dataSize,
                    rgbImage.bits(),
                    rgbImage.width(),
                    rgbImage.height(),
                    rgbImage.bytesPerLine(),
                    IPF_RGB_888,
                    0,
                    &tag);

    // Push the image into the SDK’s processing pipeline.
    m_fetcher->MyAddImageToBuffer(&data);
}

#include "videobarcodeprocessor.moc"
