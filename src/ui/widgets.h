// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef WIDGETS_H
#define WIDGETS_H

#include <QPointF>
#include <QRect>
#include <QString>

#include <functional>

class Canvas;
class Engine;

// Sprite button with idle, hover and pressed states laid out horizontally.
// A stretched button is drawn as left cap, repeated middle and right cap thirds.
class Button
{
public:
    enum State { Idle, Hover, Pressed };

    Button(int texture = -1, const QRect &sprite = QRect(), const QPointF &pos = QPointF());

    void update(Engine &engine);
    void draw(Canvas &canvas) const;

    bool clicked() const { return m_clicked; }
    State state() const { return m_state; }
    QRectF rect() const;

    QPointF pos;
    QString text;
    int stretch = 1;
    int deadBorder = 0;
    int hoverSound = -1;
    float hoverPitch = 0;
    bool selected = false;

private:
    int m_texture;
    QRect m_sprite;
    State m_state = Idle;
    bool m_clicked = false;
};

class Slider
{
public:
    explicit Slider(float value = 0, std::function<void(float)> onChanged = nullptr);

    void update(Engine &engine);
    void draw(Canvas &canvas) const;

    QPointF pos;

private:
    QRectF track() const;

    std::function<void(float)> m_onChanged;
    qreal m_thumb;
    bool m_dragging = false;
};

#endif // WIDGETS_H
