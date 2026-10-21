// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

import QtQuick 2.6
import Sailfish.Silica 1.0

CoverBackground {
    readonly property bool playing: app.score >= 0

    objectName: "defaultCover"

    CoverTemplate {
        objectName: "applicationCover"
        primaryText: playing ? app.score.toString() : "Zuma"
        secondaryText: playing ? app.levelName : "Deluxe HD"
        icon {
            source: Qt.resolvedUrl("../icons/AurZuma.png")
            sourceSize { width: icon.width; height: icon.height }
        }
    }
}
