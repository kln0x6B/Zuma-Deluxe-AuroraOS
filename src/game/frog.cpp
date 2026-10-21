// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "frog.h"

#include "assets.h"

#include <QtMath>

#include <utility>

const qreal Frog::BallOffset = 35;

namespace {

const qreal NextBallOffset = 40;
const qreal TongueOffset = 25;
const float RecoilShift = 10;
const float TongueRecoil = 40;
const float IntroTongue = 24;
const float RecoilRecovery = 1;
const float BlinkSpeed = 0.25f;
const qreal TouchRadius = 60;

}

Frog::Frog(const QPointF &position)
    : m_position(position)
    , m_forward(0, 1)
    , m_eyes(Assets::FrogTexture, QRect(0, 162, 162, 162))
{
    m_eyes.setRange(0, 4, 0);
}

bool Frog::contains(const QPointF &point) const
{
    const QPointF delta = point - m_position;
    return QPointF::dotProduct(delta, delta) < TouchRadius * TouchRadius;
}

void Frog::aim(const QPointF &target)
{
    const QPointF delta = target - m_position;
    m_rotation = qRadiansToDegrees(2 * M_PI - qAtan2(delta.x(), delta.y()));

    const qreal length = qSqrt(QPointF::dotProduct(delta, delta));
    if (length > 0)
        m_forward = delta / length;
}

void Frog::swapColors()
{
    std::swap(color, nextColor);
}

void Frog::animate(bool introEnded)
{
    if (m_shot) {
        m_shift = RecoilShift;
        m_tongue = TongueRecoil;
        m_eyes.setRange(0, 3, BlinkSpeed);
    }

    m_drawShift = qMax(m_shift, 0.0f);
    if (m_shift > 0)
        m_shift -= RecoilRecovery;
    if (m_tongue > 0)
        m_tongue -= RecoilRecovery * (TongueRecoil / RecoilShift);
    if (!introEnded)
        m_tongue = IntroTongue;

    m_eyes.angle = float(m_rotation);
    m_eyes.advance();
    if (m_eyes.frame == 0)
        m_eyes.setRange(0, 3, 0);

    m_justShot = m_shot;
    m_shot = false;
}

void Frog::draw(Canvas &canvas, bool introEnded) const
{
    const QPointF center = m_position - m_forward * m_drawShift;

    auto layer = [&](int texture, const QRect &rect, const QPointF &at, int frame = 0, float scale = 1) {
        Sprite sprite(texture, rect);
        sprite.frame = frame;
        sprite.angle = float(m_rotation);
        sprite.scale = scale;
        sprite.draw(canvas, at);
    };

    layer(Assets::FrogTexture, QRect(162, 162, 162, 162), center);
    layer(Assets::FrogTexture, QRect(0, 0, 162, 162), center);
    layer(Assets::FrogTexture, QRect(162, 0, 162, 162), center + m_forward * (TongueOffset - m_tongue));

    if (introEnded) {
        layer(Assets::ObjectsTexture, QRect(48 * color, 0, 48, 48),
              center + m_forward * (BallOffset - m_tongue), 24);
        if (!m_justShot)
            layer(Assets::FrogTexture, QRect(15 * nextColor + 234, 633, 15, 15),
                  center - m_forward * NextBallOffset, 0, 1.6f);
    }

    m_eyes.draw(canvas, center);
}
