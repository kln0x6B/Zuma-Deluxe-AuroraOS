// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "sprite.h"

Sprite::Sprite(int texture, const QRect &firstFrame)
    : texture(texture)
    , rect(firstFrame)
{
}

void Sprite::setRange(int first, int last, float speed)
{
    this->first = first;
    this->last = last;
    this->speed = speed;
    if (frame < first || frame > last)
        frame = first;
}

void Sprite::advance()
{
    if (speed == 0)
        return;

    frame += speed;
    if (frame >= last)
        frame = first;
    if (frame < first)
        frame = last - 1;
}

void Sprite::draw(Canvas &canvas, const QPointF &center) const
{
    const QRect source = rect.translated(0, rect.height() * int(frame));
    canvas.drawCentered(texture, source, center, scale, angle, color);
}
