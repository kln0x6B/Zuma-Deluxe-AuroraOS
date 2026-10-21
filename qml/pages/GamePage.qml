// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

import QtQuick 2.6
import Sailfish.Silica 1.0
import ru.kln0x6b.AurZuma 1.0

Page {
    id: page

    property alias levelName: gameView.levelName
    property alias score: gameView.score

    objectName: "gamePage"
    allowedOrientations: Orientation.LandscapeMask
    backNavigation: false
    showNavigationIndicator: false

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    GameView {
        id: gameView

        objectName: "gameView"
        anchors.fill: parent
        active: Qt.application.active && page.status === PageStatus.Active

        onQuitRequested: Qt.quit()
    }
}
