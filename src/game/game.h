// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef GAME_H
#define GAME_H

#include "assets.h"
#include "ballchain.h"
#include "bullets.h"
#include "effects.h"
#include "frog.h"
#include "levels.h"
#include "ui/dialog.h"

#include <memory>

class Engine;

// A single attempt at a level, from the sparkle intro to the results dialog.
class Game
{
public:
    Game(Engine &engine, const LevelLibrary &levels, int levelIndex, int difficulty);
    ~Game();

    bool isValid() const { return m_level->isValid(); }
    bool isFinished() const { return m_finished; }
    int score() const { return m_score; }
    int levelIndex() const { return m_levelIndex; }
    int difficulty() const { return m_difficulty; }
    QString levelName() const { return m_level->info().displayName; }

    void pause();

    void update();
    void draw(Canvas &canvas) const;

private:
    // Sparkles running along the path before the level and bonus bursts after it.
    struct Sweep
    {
        Particles particles;
        QPointF head;
        float progress = 0;
        float lastSpawn = 0;
        bool started = false;
    };

    struct Treasure
    {
        int point = 0;
        bool active = false;
        bool blinking = false;
        bool fading = false;
        double since = 0;
        Sprite sprite;
    };

    void step();
    void animate();
    bool playfieldVisible() const;
    bool hasDialog() const;

    bool takeShot();
    void shoot();
    int randomChainColor() const;
    void keepFrogColorsInChain();
    void updateChainSpeed();
    void generateChain();

    void updateIntro();
    void updateOutro();
    void advanceSweep(float speed);
    void awardOutroBonus();

    bool updateEnding();
    bool updatePauseMenu();
    void checkWin();
    bool checkLoss();
    void saveRecord();
    void finishLevel(Assets::Music music);
    static QStringList statNames();
    QStringList statValues() const;

    void updateTreasure();
    void animateTreasure();
    bool treasureVisible() const;
    QPointF treasurePosition() const;

    void drawHud(Canvas &canvas) const;
    void drawPit(Canvas &canvas) const;
    void drawIntro(Canvas &canvas) const;
    void drawTreasure(Canvas &canvas) const;

    Engine &m_engine;
    int m_levelIndex;
    int m_difficulty;
    std::unique_ptr<Level> m_level;
    LevelSettings m_settings;
    Frog m_frog;
    BallChain m_chain;
    Bullets m_bullets;
    FloatingTexts m_texts;
    Sweep m_sweep;
    Treasure m_treasure;
    Sprite m_pit;
    Button m_menuButton;
    QRect m_gauge;

    std::unique_ptr<Dialog> m_menu;
    std::unique_ptr<Dialog> m_stats;
    std::unique_ptr<Dialog> m_gameOver;

    int m_score = 0;
    int m_coins = 0;
    int m_elapsed = 0;
    double m_startTime = 0;
    double m_lightTrailTime = 0;
    float m_lightTrailPitch = 0;
    float m_headerScale = 0;
    float m_subheaderScale = 0;
    float m_headPos = 0;
    QPointF m_headPoint;

    bool m_introEnded = false;
    bool m_outroEnded = false;
    bool m_firstWave = true;
    bool m_won = false;
    bool m_lost = false;
    bool m_finished = false;
};

#endif // GAME_H
