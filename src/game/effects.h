// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef EFFECTS_H
#define EFFECTS_H

#include "sprite.h"

#include <QVector>

// Score popups that rise and disappear after a second.
class FloatingTexts
{
public:
    void spawn(const QString &text, const QPointF &pos, int ballColor, double now);
    void update(double now);
    void draw(Canvas &canvas) const;

private:
    struct Item
    {
        QString text;
        QPointF pos;
        Color color;
        double born;
    };

    QVector<Item> m_items;
};

// One-shot animations sharing a sprite, each playing to the given last frame.
class Particles
{
public:
    explicit Particles(const Sprite &sprite = Sprite()) : m_sprite(sprite) {}

    void spawn(const QPointF &pos);
    void update(float lastFrame, float speed);
    void draw(Canvas &canvas) const;
    bool isActive() const { return !m_items.isEmpty(); }

private:
    struct Item
    {
        QPointF pos;
        float frame;
    };

    Sprite m_sprite;
    QVector<Item> m_items;
};

#endif // EFFECTS_H
