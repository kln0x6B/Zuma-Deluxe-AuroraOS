// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "effects.h"

#include "assets.h"

namespace {

const double TextLifetime = 1.0;
const qreal TextRiseSpeed = 2;

const Color TextColors[] = {
    Color(0x00, 0xA9, 0xFF),
    Color(0x00, 0xFF, 0x00),
    Color(0xFF, 0xFF, 0x00),
    Color(0xFF, 0x00, 0x00),
    Color(0xFF, 0x00, 0xFF),
    Color(0xFF, 0xFF, 0xFF),
};

}

void FloatingTexts::spawn(const QString &text, const QPointF &pos, int ballColor, double now)
{
    m_items.append({ text, pos, TextColors[qBound(0, ballColor, 5)], now });
}

void FloatingTexts::update(double now)
{
    for (int i = m_items.size() - 1; i >= 0; --i) {
        if (now - m_items.at(i).born > TextLifetime)
            m_items.remove(i);
        else
            m_items[i].pos.ry() -= TextRiseSpeed;
    }
}

void FloatingTexts::draw(Canvas &canvas) const
{
    for (const Item &item : m_items)
        canvas.text(Assets::CancunFloat14, item.text, item.pos, item.color, TextShadow);
}

void Particles::spawn(const QPointF &pos)
{
    m_items.append({ pos, 0.0f });
}

void Particles::update(float lastFrame, float speed)
{
    for (int i = m_items.size() - 1; i >= 0; --i) {
        if (m_items.at(i).frame > lastFrame)
            m_items.remove(i);
        else
            m_items[i].frame += speed;
    }
}

void Particles::draw(Canvas &canvas) const
{
    Sprite sprite = m_sprite;
    for (const Item &item : m_items) {
        sprite.frame = int(item.frame);
        sprite.draw(canvas, item.pos);
    }
}
