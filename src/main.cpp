// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "gameview.h"

#include <QtQuick>

#include <auroraapp.h>

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> application(Aurora::Application::application(argc, argv));
    application->setOrganizationName(QStringLiteral("ru.kln0x6b"));
    application->setApplicationName(QStringLiteral("AurZuma"));

    qmlRegisterType<GameView>("ru.kln0x6b.AurZuma", 1, 0, "GameView");

    QScopedPointer<QQuickView> view(Aurora::Application::createView());
    view->setSource(Aurora::Application::pathTo(QStringLiteral("qml/AurZuma.qml")));
    view->show();

    return application->exec();
}
