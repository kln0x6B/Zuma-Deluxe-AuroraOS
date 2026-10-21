// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef BALLCHAIN_H
#define BALLCHAIN_H

#include "levels.h"
#include "sprite.h"

#include <QVector>

class Engine;
class FloatingTexts;

struct Ball
{
    void resetSprite();

    int color = 0;
    float pos = 0;
    float speed = 0;
    QPointF point;
    qreal angle = 0;
    int combo = 0;
    bool goBack = false;
    bool inserted = false;
    bool exploding = false;
    bool single = false;
    bool inTunnel = false;
    bool priority = false;
    bool fadingIn = false;
    Sprite sprite;
};

Q_DECLARE_TYPEINFO(Ball, Q_MOVABLE_TYPE);

// The rolling chain; index 0 is the head closest to the skull pit.
class BallChain
{
public:
    static const float Pad;
    static const float RollingToPitSpeed;

    struct Stats
    {
        int maxCombo = 0;
        int totalCombos = 0;
        int maxChainBonus = 0;
    };

    BallChain(Engine &engine, const LevelPath &path, const LevelSettings &settings);

    int size() const { return m_balls.size(); }
    bool isEmpty() const { return m_balls.isEmpty(); }
    const Ball &ball(int index) const { return m_balls.at(index); }
    Ball &ball(int index) { return m_balls[index]; }
    bool hasColor(int color) const { return m_present.at(color); }
    int colors() const { return m_settings.colors; }
    const Stats &stats() const { return m_stats; }

    void append();
    void insert(int index, int color, bool behind);
    int lastMovingBall() const;
    void startGlow(double now);

    void update(int &score, FloatingTexts &texts);
    void animate();
    void draw(Canvas &canvas, bool priority) const;

    float speed = 0;
    int chainBonus = 0;
    bool generating = true;
    bool endReached = false;

private:
    bool collidesFront(int index, float distance) const;
    bool collidesBack(int index, float distance) const;
    void findGroup(int index, int &start, int &end) const;
    void remove(int start, int end);
    void explode(int start, int end);
    void resolveCollision(int index, int &score, FloatingTexts &texts);
    void resolveInsertion(int index, int &score, FloatingTexts &texts);
    void announce(int start, int end, const QString &text, FloatingTexts &texts);
    void place(Ball &ball);
    void updateSpeeds(int &score, FloatingTexts &texts);
    void updatePositions(int &score, FloatingTexts &texts);
    void updateColors();
    void updateCombos();

    Engine &m_engine;
    const LevelPath &m_path;
    LevelSettings m_settings;
    QVector<Ball> m_balls;
    QVector<bool> m_present;
    Stats m_stats;
    Sprite m_shadow;
    bool m_glowing = false;
    double m_glowStart = 0;
};

#endif // BALLCHAIN_H
