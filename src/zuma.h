// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef ZUMA_H
#define ZUMA_H

#include "engine/engine.h"
#include "game/levels.h"
#include "ui/menuscreen.h"

#include <QObject>

class Game;

// Top-level flow: disclaimer, menus and the running game.
class Zuma : public QObject, public Scene
{
    Q_OBJECT

public:
    explicit Zuma(Engine &engine, QObject *parent = nullptr);
    ~Zuma() override;

    QString levelName() const;
    int score() const;

    void setPaused(bool paused);

    void update() override;
    void draw(Canvas &canvas) const override;

signals:
    void quitRequested();
    void statusChanged();

private:
    enum class State { Broken, Splash, Menu, Playing };

    void startGame();
    void leaveGame();

    Engine &m_engine;
    LevelLibrary m_levels;
    MenuScreen m_menu;
    std::unique_ptr<Game> m_game;
    State m_state = State::Broken;
    double m_splashStart = 0;
    int m_reportedScore = -1;
};

#endif // ZUMA_H
