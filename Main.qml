import QtQuick 2.15
import QtQuick.Window 2.15
import QtMultimedia 5.15
import com.mycompany.videobarcode 1.0

Window {
    width: 640
    height: 480
    visible: true
    title: "Barcode Scanner"

    VideoBarcodeProcessor {
        id: barcodeProcessor
        videoSink: viewFinder.videoSink()
        onBarcodeResult: {
            console.log("Barcode result:", barcodeResult)
        }
    }

    CaptureSession {
        id: captureSession
        camera: Camera { id: camera }
        videoOutput: viewFinder
    }

    VideoOutput {
        id: viewFinder
        anchors.fill: parent
        // The videoSink() property is provided by the VideoOutput element.
    }

    Component.onCompleted: {
        camera.start()
    }
}
