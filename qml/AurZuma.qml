// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

import QtQuick 2.6
import Sailfish.Silica 1.0
import "pages"

ApplicationWindow {
    id: app

    property string levelName
    property int score: -1
    property string levelTitle
    property url thumbnail

    objectName: "applicationWindow"
    allowedOrientations: Orientation.LandscapeMask

    // Display modes appeared after OS 5.1, where the window is full screen anyway
    Component.onCompleted: {
        if (app.hasOwnProperty("displayMode"))
            app.displayMode = ApplicationDisplayMode.FillScreen
    }

    initialPage: Component {
        GamePage {
            id: gamePage

            Binding { target: app; property: "levelName"; value: gamePage.levelName }
            Binding { target: app; property: "score"; value: gamePage.score }
            Binding { target: app; property: "levelTitle"; value: gamePage.levelTitle }
            Binding { target: app; property: "thumbnail"; value: gamePage.thumbnail }
        }
    }

    cover: Qt.resolvedUrl("cover/DefaultCoverPage.qml")
}
