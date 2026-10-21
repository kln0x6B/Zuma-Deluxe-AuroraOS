// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef BULLETS_H
#define BULLETS_H

#include "sprite.h"

#include <QVector>

class BallChain;
class Frog;
class LevelPath;

class Bullets
{
public:
    struct Bullet
    {
        QPointF pos;
        QPointF direction;
        int color;
        Sprite sprite;
    };

    void fire(const Frog &frog);
    // Returns true when a bullet left the visible area.
    bool update(const QRectF &visible);
    void collide(BallChain &chain, const LevelPath &path);
    void draw(Canvas &canvas) const;

    QVector<Bullet> &items() { return m_bullets; }

private:
    QVector<Bullet> m_bullets;
};

#endif // BULLETS_H
