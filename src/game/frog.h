// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef FROG_H
#define FROG_H

#include "sprite.h"

class Frog
{
public:
    static const qreal BallOffset;

    explicit Frog(const QPointF &position);

    const QPointF &position() const { return m_position; }
    const QPointF &forward() const { return m_forward; }
    qreal rotation() const { return m_rotation; }
    bool isRecoiling() const { return int(m_shift) != 0; }
    bool contains(const QPointF &point) const;

    int color = 0;
    int nextColor = 1;

    void aim(const QPointF &target);
    void swapColors();
    void markShot() { m_shot = true; }

    void animate(bool introEnded);
    void draw(Canvas &canvas, bool introEnded) const;

private:
    QPointF m_position;
    QPointF m_forward;
    qreal m_rotation = 0;
    float m_shift = 0;
    float m_drawShift = 0;
    float m_tongue = 0;
    bool m_shot = false;
    bool m_justShot = false;
    Sprite m_eyes;
};

#endif // FROG_H
