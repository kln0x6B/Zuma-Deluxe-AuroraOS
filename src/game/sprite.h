// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef SPRITE_H
#define SPRITE_H

#include "engine/canvas.h"

#include <QRect>

// Frame animation laid out vertically on a sprite sheet, starting from the first frame rectangle.
struct Sprite
{
    Sprite() = default;
    Sprite(int texture, const QRect &firstFrame);

    void setRange(int first, int last, float speed);
    void advance();
    void draw(Canvas &canvas, const QPointF &center) const;

    int texture = -1;
    QRect rect;
    float frame = 0;
    float speed = 0;
    int first = 0;
    int last = 0;
    float scale = 1;
    float angle = 0;
    Color color;
};

#endif // SPRITE_H
