// The content of this file is shamelessly taken from
// https://forum.qt.io/topic/52161/properly-scaling-svg-images/6
//
// This QML file solves the problem of the weird behavior that SVG files
// are first rendered into a pixel graphic before they are scaled, which
// leads to pixelated or blurred images.
//
// One solution would be to change the image size in each SVG file such that
// the display will only scale down the graphics.
// However, one does not want to change every SVG file (and keep track of
// this issue for newly introduced SVG files until this QML issue is fixed).
// The solution that we use here is to trick the renderer by pretending that
// we have a big SVG image.

import QtQuick 2.12
import QtQuick.Controls 2.12

Image {
  sourceSize: Qt.size(Math.max(hiddenImg.sourceSize.width, 250),
                      Math.max(hiddenImg.sourceSize.height, 250))

  Image {
    id: hiddenImg
    source: parent.source
    width: 0
    height: 0
  }
}

