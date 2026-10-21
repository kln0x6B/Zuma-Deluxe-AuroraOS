// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "widgets.h"

#include "engine/engine.h"
#include "game/assets.h"

#include <utility>

namespace {

const QRect SliderTrack(0, 365, 307, 49);
const QRect SliderThumb(0, 414, 61, 45);
const qreal ThumbMax = SliderTrack.width() - SliderThumb.width() * 1.125;
const qreal TouchSlop = 12;

}

Button::Button(int texture, const QRect &sprite, const QPointF &pos)
    : pos(pos)
    , m_texture(texture)
    , m_sprite(sprite)
{
}

QRectF Button::rect() const
{
    const int width = m_sprite.width() + m_sprite.width() / 3 * (stretch - 1);
    return QRectF(pos, QSizeF(width, m_sprite.height()));
}

void Button::update(Engine &engine)
{
    const Pointer &pointer = engine.pointer();
    const QRectF area = rect().adjusted(deadBorder, deadBorder, -deadBorder, -deadBorder);

    if (!pointer.over(area)) {
        m_state = Idle;
        m_clicked = false;
        return;
    }

    if (m_state == Idle && hoverSound >= 0)
        engine.audio().playPitched(hoverSound, hoverPitch);

    if (pointer.down) {
        if (m_state != Pressed)
            engine.audio().play(Assets::ButtonSound);
        m_state = Pressed;
        m_clicked = false;
    } else {
        m_clicked = m_state == Pressed;
        m_state = Hover;
    }
}

void Button::draw(Canvas &canvas) const
{
    const int width = m_sprite.width();
    const int height = m_sprite.height();

    if (stretch == 1) {
        const int column = m_state == Hover ? 1 : (m_state == Pressed || selected ? 2 : 0);
        canvas.draw(m_texture, m_sprite.translated(width * column, 0), QRectF(pos, QSizeF(width, height)));
    } else {
        const int slice = width / 3;
        const int column = m_state == Hover ? 1 : (m_state == Pressed ? 2 : 0);
        for (int i = 0; i < stretch + 2; ++i) {
            const int part = i == 0 ? 0 : (i == stretch + 1 ? 2 : 1);
            const QRect source(m_sprite.x() + width * column + slice * part, m_sprite.y(), slice, height);
            canvas.draw(m_texture, source, QRectF(pos.x() + slice * i, pos.y(), slice, height));
        }
    }

    if (text.isEmpty())
        return;

    const int center = int(pos.x()) + (width + width / 3 * stretch) / 2 - 24;
    const QPointF shift = m_state == Pressed ? QPointF(-2, 2) : QPointF();
    const Color color = m_state == Idle ? Color(255, 255, 0) : Color();
    canvas.text(Assets::Cancun10, text, QPointF(center, pos.y() + 18) + shift, color, TextShadow | TextCenter);
}

Slider::Slider(float value, std::function<void(float)> onChanged)
    : m_onChanged(std::move(onChanged))
    , m_thumb(ThumbMax * value)
{
}

QRectF Slider::track() const
{
    return QRectF(pos, SliderTrack.size());
}

void Slider::update(Engine &engine)
{
    const Pointer &pointer = engine.pointer();
    if (pointer.pressed && pointer.over(track().adjusted(0, -TouchSlop, 0, TouchSlop)))
        m_dragging = true;
    if (!pointer.down)
        m_dragging = false;
    if (!m_dragging)
        return;

    m_thumb = qBound<qreal>(0, pointer.pos.x() - pos.x() - SliderThumb.width() / 2, ThumbMax);
    m_onChanged(float(m_thumb / ThumbMax));
}

void Slider::draw(Canvas &canvas) const
{
    canvas.draw(Assets::MenuTexture, SliderTrack, track());
    canvas.draw(Assets::MenuTexture, SliderThumb,
                QRectF(pos.x() + m_thumb, pos.y(), SliderThumb.width(), SliderTrack.height()));
}
