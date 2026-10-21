// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "bullets.h"

#include "assets.h"
#include "ballchain.h"
#include "engine/engine.h"
#include "frog.h"

namespace {

const qreal Speed = 15;
const qreal HitDistanceSquared = 2300;
const qreal ScreenMargin = 50;
const int InsertProbe = 32;

qreal distanceSquared(const QPointF &a, const QPointF &b)
{
    const QPointF delta = a - b;
    return QPointF::dotProduct(delta, delta);
}

// Compares the bullet with points half a ball ahead of and behind the hit ball along the path.
bool insertsBehind(const QPointF &bullet, const Ball &ball, const LevelPath &path)
{
    if (ball.pos - (InsertProbe - 1) < 0)
        return false;

    const int index = int(ball.pos);
    const QPointF front = ball.point + (path.point(index + InsertProbe) - path.point(index)) * 0.5;
    const QPointF back = ball.point - (path.point(index + 1) - path.point(index + 1 - InsertProbe)) * 0.5;
    return distanceSquared(front, bullet) > distanceSquared(back, bullet);
}

}

void Bullets::fire(const Frog &frog)
{
    Bullet bullet;
    bullet.pos = frog.position() + frog.forward() * Frog::BallOffset;
    bullet.direction = frog.forward();
    bullet.color = frog.color;
    bullet.sprite = Sprite(Assets::ObjectsTexture, QRect(48 * frog.color, 0, 48, 48));
    bullet.sprite.setRange(24, 47, 0);
    bullet.sprite.angle = float(frog.rotation());
    m_bullets.append(bullet);
}

bool Bullets::update(const QRectF &visible)
{
    const QRectF screen = visible.adjusted(-ScreenMargin, -ScreenMargin, ScreenMargin, ScreenMargin);

    bool missed = false;
    for (int i = m_bullets.size() - 1; i >= 0; --i) {
        if (!screen.contains(m_bullets.at(i).pos)) {
            m_bullets.remove(i);
            missed = true;
        }
    }

    for (Bullet &bullet : m_bullets)
        bullet.pos += bullet.direction * Speed;
    return missed;
}

void Bullets::collide(BallChain &chain, const LevelPath &path)
{
    for (int i = m_bullets.size() - 1; i >= 0; --i) {
        const Bullet &bullet = m_bullets.at(i);
        for (int hit = 0; hit < chain.size(); ++hit) {
            const Ball &ball = chain.ball(hit);
            if (ball.exploding || ball.inTunnel || distanceSquared(bullet.pos, ball.point) >= HitDistanceSquared)
                continue;

            const bool behind = insertsBehind(bullet.pos, ball, path);
            chain.insert(hit + (behind ? 1 : 0), bullet.color, behind);
            m_bullets.remove(i);
            break;
        }
    }
}

void Bullets::draw(Canvas &canvas) const
{
    for (const Bullet &bullet : m_bullets)
        bullet.sprite.draw(canvas, bullet.pos);
}
