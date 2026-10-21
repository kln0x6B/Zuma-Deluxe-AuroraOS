// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "ballchain.h"

#include "assets.h"
#include "effects.h"
#include "engine/engine.h"

const float BallChain::Pad = 32.0f;
const float BallChain::RollingToPitSpeed = 32.0f;

namespace {

const float PadRough = BallChain::Pad + 2.0f;
const float InsertionSpeed = 4.0f;
const float Acceleration = 0.05f;
const float Friction = 0.04f;
const float Deceleration = 0.4f;
const float WeightRatio = 2.5f;
const float MaxBackSpeed = -10.0f;
const float ExplosionSpeed = 0.5f;
const int ExplosionFrames = 16;
const double GlowDuration = 0.5;
const int MaxColors = 6;

const Color ExplosionTints[MaxColors] = {
    Color(0x00, 0xFE, 0xFD),
    Color(0x00, 0xFA, 0x44),
    Color(0xFD, 0xFD, 0x06),
    Color(0xF9, 0x6F, 0x4C),
    Color(0xFD, 0x88, 0xF8),
    Color(0xFD, 0xEA, 0xBB),
};

float sign(float value)
{
    return value > 0 ? 1.0f : (value < 0 ? -1.0f : 0.0f);
}

}

void Ball::resetSprite()
{
    sprite = Sprite(Assets::ObjectsTexture, QRect(48 * color, 0, 48, 48));
    if (fadingIn)
        sprite.color.a = 0;
    sprite.setRange(0, color == 0 ? 47 : 50, 0);
}

BallChain::BallChain(Engine &engine, const LevelPath &path, const LevelSettings &settings)
    : speed(settings.speed)
    , m_engine(engine)
    , m_path(path)
    , m_settings(settings)
    , m_present(MaxColors, false)
    , m_shadow(Assets::ObjectsTexture, QRect(134, 2595, 48, 50))
{
    m_settings.colors = qBound(1, m_settings.colors, MaxColors);
    m_shadow.color.a = 128;
    append();
}

void BallChain::append()
{
    auto otherColor = [this](int color) {
        int other = color;
        while (other == color && m_settings.colors > 1)
            other = Engine::random(0, m_settings.colors - 1);
        return other;
    };

    Ball ball;
    ball.point = m_path.start();

    if (m_balls.isEmpty()) {
        ball.color = Engine::random(0, m_settings.colors - 1);
    } else if (m_balls.last().single) {
        ball.color = otherColor(m_balls.last().color);
    } else if (Engine::random(1, 100) < m_settings.singleChance) {
        ball.color = otherColor(m_balls.last().color);
        ball.single = true;
    } else if (Engine::random(1, 100) < m_settings.repeatChance) {
        int start, end;
        findGroup(m_balls.size() - 1, start, end);
        const int lastColor = m_balls.last().color;
        ball.color = end - start + 1 < 4 ? lastColor : otherColor(lastColor);
    } else {
        ball.color = Engine::random(0, m_settings.colors - 1);
    }

    ball.resetSprite();
    m_balls.append(ball);
}

void BallChain::insert(int index, int color, bool behind)
{
    const int hit = behind ? index - 1 : index;
    Ball ball = m_balls.at(index < m_balls.size() ? index : hit);
    if (behind) {
        ball.pos = m_balls.at(hit).pos;
        ball.point = m_balls.at(hit).point;
    }
    ball.color = color;
    ball.exploding = false;
    ball.inserted = true;
    ball.resetSprite();

    m_balls.insert(index, ball);
    m_engine.audio().play(Assets::BallInsertSound);
}

int BallChain::lastMovingBall() const
{
    for (int i = m_balls.size() - 1; i >= 0; --i) {
        if (!collidesFront(i, PadRough))
            return i;
    }
    return m_balls.size() - 1;
}

void BallChain::startGlow(double now)
{
    m_glowing = true;
    m_glowStart = now;
}

bool BallChain::collidesFront(int index, float distance) const
{
    return index > 0 && m_balls.at(index - 1).pos - m_balls.at(index).pos < distance;
}

bool BallChain::collidesBack(int index, float distance) const
{
    return index < m_balls.size() - 1 && m_balls.at(index).pos - m_balls.at(index + 1).pos < distance;
}

// Bounds of the touching run of same-colored balls around the given one.
void BallChain::findGroup(int index, int &start, int &end) const
{
    auto sameAs = [this](int a, int b) {
        return m_balls.at(a).color == m_balls.at(b).color
                && m_balls.at(a).exploding == m_balls.at(b).exploding;
    };

    start = end = index;
    while (end < m_balls.size() - 1 && collidesBack(end, PadRough) && sameAs(end, end + 1))
        ++end;
    while (start > 0 && collidesFront(start, PadRough) && sameAs(start, start - 1))
        --start;
}

void BallChain::remove(int start, int end)
{
    m_balls.remove(start, end - start + 1);
}

void BallChain::explode(int start, int end)
{
    int combo = 0;
    for (int i = start; i <= end; ++i) {
        Ball &ball = m_balls[i];
        combo = qMax(combo, ball.combo);
        ball.sprite = Sprite(Assets::ObjectsTexture, QRect(395, 0, 105, 120));
        ball.sprite.color = ExplosionTints[ball.color];
        ball.sprite.setRange(0, ExplosionFrames, ExplosionSpeed);
        ball.exploding = true;
    }

    const int level = qMin(combo, 4);
    m_engine.audio().play(Assets::MatchSound1 + level);
    m_engine.audio().playPitched(Assets::ChimeSound, level * 2.0f);
}

void BallChain::announce(int start, int end, const QString &text, FloatingTexts &texts)
{
    const int middle = end - (end - start + 1) / 2;
    texts.spawn(text, m_balls.at(middle).point, m_balls.at(start).color, m_engine.time());
}

// A ball rolling back has hit the chain behind it: matches there score combos.
void BallChain::resolveCollision(int index, int &score, FloatingTexts &texts)
{
    int start, end;
    findGroup(index, start, end);
    if (end - start + 1 < 3) {
        m_balls[index + 1].combo = 0;
        return;
    }

    explode(start, end);

    int combo = 0;
    for (int i = start; i <= end && !combo; ++i)
        combo = m_balls.at(i).combo;
    if (end + 1 < m_balls.size())
        m_balls[end + 1].combo = combo + 1;

    const int points = (end - start + 1) * 10 + combo * 100;
    score += points;
    if (combo)
        ++m_stats.totalCombos;
    m_stats.maxCombo = qMax(m_stats.maxCombo, combo);

    announce(start, end, combo ? QStringLiteral("+%1\nCOMBO x%2").arg(points).arg(combo)
                               : QStringLiteral("+%1").arg(points), texts);
}

// A shot ball has settled into the chain: matches there grow the chain bonus.
void BallChain::resolveInsertion(int index, int &score, FloatingTexts &texts)
{
    int start, end;
    findGroup(index, start, end);
    if (end - start + 1 < 3) {
        chainBonus = 0;
        return;
    }

    explode(start, end);
    ++chainBonus;
    m_stats.maxChainBonus = qMax(m_stats.maxChainBonus, chainBonus);

    const bool bonus = chainBonus >= 5;
    int points = (end - start + 1) * 10;
    if (bonus)
        points += 100 + (chainBonus - 5) * 10;
    score += points;
    if (end + 1 < m_balls.size())
        m_balls[end + 1].combo = 1;

    announce(start, end, bonus ? QStringLiteral("+%1\nCHAIN BONUS x%2").arg(points).arg(chainBonus)
                               : QStringLiteral("+%1").arg(points), texts);
}

void BallChain::update(int &score, FloatingTexts &texts)
{
    updateSpeeds(score, texts);
    updatePositions(score, texts);

    for (int i = 0; i < m_balls.size(); ++i) {
        const Ball &ball = m_balls.at(i);
        if (ball.exploding && int(ball.sprite.frame) == ball.sprite.last - 1) {
            int start, end;
            findGroup(i, start, end);
            remove(start, end);
        }
    }

    updateColors();
    updateCombos();

    if (m_glowing && m_engine.time() - m_glowStart > GlowDuration)
        m_glowing = false;
}

void BallChain::updateSpeeds(int &score, FloatingTexts &texts)
{
    const int tail = m_balls.size() - 1;
    for (int i = tail; i >= 0; --i) {
        Ball &ball = m_balls[i];

        if (i == tail) {
            ball.speed = speed == RollingToPitSpeed ? RollingToPitSpeed : ball.speed + Acceleration;
            ball.speed = qMin(ball.speed, speed);
        } else if (!collidesBack(i, PadRough)) {
            if (ball.color == m_balls.at(i + 1).color && !endReached) {
                ball.speed = qMax(ball.speed - Deceleration, MaxBackSpeed);
                ball.goBack = true;
            } else {
                ball.speed -= qMin(qAbs(ball.speed), Friction) * sign(ball.speed);
            }
        } else if (!ball.goBack) {
            ball.speed = m_balls.at(i + 1).speed;
        } else {
            // The impact is passed on to the balls behind, damped at every step as in the original
            for (int j = i; j <= tail; ++j) {
                m_balls[j].speed = ball.speed / WeightRatio;
                if (!collidesBack(j, PadRough)) {
                    m_engine.audio().play(Assets::BallClickSound);
                    break;
                }
            }
            ball.speed = 0;

            if (!ball.exploding && !endReached)
                resolveCollision(i, score, texts);
            ball.goBack = false;
        }
    }
}

void BallChain::updatePositions(int &score, FloatingTexts &texts)
{
    for (int i = m_balls.size() - 1; i >= 0; --i) {
        if (m_balls.at(i).pos < 0) {
            if (m_balls.size() > 1) {
                remove(i, i);
                continue;
            }
            m_balls[i].pos = 0;
        }

        if (int(m_balls.at(i).pos) >= m_path.length()) {
            remove(i, i);
            generating = false;
            endReached = true;
            continue;
        }

        Ball &ball = m_balls[i];
        ball.pos += ball.speed;

        if (i < m_balls.size() - 1) {
            if (!m_balls.at(i + 1).inserted) {
                while (collidesBack(i, Pad))
                    ball.pos += 1;
            } else if (collidesBack(i, Pad)) {
                ball.pos += InsertionSpeed;
            } else {
                resolveInsertion(i + 1, score, texts);
                m_balls[i + 1].inserted = false;
            }
        }

        if (i == 0 && ball.inserted && !ball.exploding) {
            resolveInsertion(0, score, texts);
            ball.inserted = false;
        }

        place(ball);
    }
}

void BallChain::place(Ball &ball)
{
    ball.point = m_path.at(ball.pos);

    const int passed = int(ball.pos) - 1;
    if (passed >= 0) {
        const LevelPath::Step &step = m_path.step(passed);
        ball.angle = step.angle;
        ball.inTunnel = step.tunnel;
        ball.priority = step.priority;
    }
}

void BallChain::updateColors()
{
    m_present.fill(false);
    for (const Ball &ball : m_balls)
        m_present[ball.color] = true;
}

void BallChain::updateCombos()
{
    if (m_balls.isEmpty())
        return;

    m_balls[0].combo = 0;
    for (int i = 1; i < m_balls.size(); ++i) {
        const Ball &front = m_balls.at(i - 1);
        if (!front.exploding && (collidesFront(i, Pad) || front.color != m_balls.at(i).color))
            m_balls[i].combo = 0;
    }
}

void BallChain::animate()
{
    for (Ball &ball : m_balls) {
        if (!ball.exploding)
            ball.sprite.frame = qMax(0, int(ball.pos)) % ball.sprite.last;
        if (ball.fadingIn) {
            ball.sprite.color.a = quint8(qMin(255, ball.sprite.color.a + 15));
            ball.fadingIn = ball.sprite.color.a < 255;
        }
        ball.sprite.angle = float(ball.angle - 90);
        ball.sprite.advance();
    }
}

void BallChain::draw(Canvas &canvas, bool priority) const
{
    for (const Ball &ball : m_balls) {
        if (ball.priority != priority || ball.exploding)
            continue;
        Sprite shadow = m_shadow;
        shadow.color.a = qMin<quint8>(ball.sprite.color.a, 128);
        shadow.draw(canvas, ball.point + QPointF(-4, 6));
    }

    for (const Ball &ball : m_balls) {
        if (ball.priority == priority)
            ball.sprite.draw(canvas, ball.point);
    }

    if (!m_glowing)
        return;

    Sprite glow(Assets::ObjectsTexture, QRect(182, 2595, 48, 50));
    glow.color.a = 128;
    for (const Ball &ball : m_balls) {
        if (ball.priority == priority && !ball.exploding)
            glow.draw(canvas, ball.point);
    }
}
