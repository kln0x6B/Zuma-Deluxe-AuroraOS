// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef MENUSCREEN_H
#define MENUSCREEN_H

#include "dialog.h"

class LevelLibrary;

class MenuScreen
{
public:
    enum class Action { None, Play, Quit };

    MenuScreen(Engine &engine, const LevelLibrary &levels);
    ~MenuScreen();

    void showMain();
    void showGauntlet();

    Action update();
    void draw(Canvas &canvas) const;

    int level() const { return m_level; }
    int difficulty() const { return m_difficulty; }

private:
    enum class Room { Main, Gauntlet };

    void setRoom(Room room, const QRect &background, const QRect &sky);
    Action handleMain(int button);
    Action handleGauntlet(int button);
    void drawMainDecor(Canvas &canvas) const;
    void drawRecords(Canvas &canvas) const;

    Engine &m_engine;
    const LevelLibrary &m_levels;
    Room m_room = Room::Main;
    QRect m_background;
    QRect m_sky;
    int m_skyOffset = 0;
    QVector<Button> m_buttons;
    std::unique_ptr<Dialog> m_dialog;
    int m_level = 0;
    int m_difficulty = 0;
};

#endif // MENUSCREEN_H
