// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "zuma.h"

#include "game/assets.h"
#include "game/game.h"

namespace {

const double SplashDuration = 2.0;

}

Zuma::Zuma(Engine &engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
    , m_menu(engine, m_levels)
{
    if (Assets::load(engine) && m_levels.load(engine.path(QStringLiteral("levels/levels.xml"))))
        m_state = State::Splash;
    else
        qWarning("Game data is incomplete");
}

Zuma::~Zuma() = default;

QString Zuma::levelName() const
{
    return m_game ? m_game->levelName() : QString();
}

int Zuma::score() const
{
    return m_game ? m_game->score() : -1;
}

QString Zuma::levelTitle() const
{
    return m_game ? QStringLiteral("LEVEL %1-%2").arg(m_game->levelIndex() + 1).arg(m_game->difficulty() + 1)
                  : QString();
}

// The level being played or selected in the menu
QString Zuma::thumbnail() const
{
    return m_engine.path(Assets::thumbnailPath(m_game ? m_game->levelIndex() : m_menu.level()));
}

void Zuma::setPaused(bool paused)
{
    if (paused && m_game)
        m_game->pause();
}

void Zuma::update()
{
    switch (m_state) {
    case State::Broken:
        break;
    case State::Splash:
        if (m_engine.time() - m_splashStart > SplashDuration || m_engine.pointer().released) {
            m_state = State::Menu;
            m_menu.showMain();
        }
        break;
    case State::Menu:
        switch (m_menu.update()) {
        case MenuScreen::Action::Play:
            startGame();
            break;
        case MenuScreen::Action::Quit:
            emit quitRequested();
            break;
        case MenuScreen::Action::None:
            break;
        }
        break;
    case State::Playing:
        m_game->update();
        if (m_game->isFinished())
            leaveGame();
        break;
    }

    if (score() != m_reportedScore || thumbnail() != m_reportedThumbnail) {
        m_reportedScore = score();
        m_reportedThumbnail = thumbnail();
        emit statusChanged();
    }
}

void Zuma::startGame()
{
    m_game.reset(new Game(m_engine, m_levels, m_menu.level(), m_menu.difficulty()));
    if (m_game->isValid()) {
        m_state = State::Playing;
        return;
    }

    qWarning("Unable to load level %d", m_menu.level());
    leaveGame();
}

void Zuma::leaveGame()
{
    m_game.reset();
    m_state = State::Menu;
    m_menu.showGauntlet();
}

void Zuma::draw(Canvas &canvas) const
{
    switch (m_state) {
    case State::Broken:
        break;
    case State::Splash:
        canvas.drawImage(Assets::DisclaimerTexture, QPointF(640, 360));
        break;
    case State::Menu:
        m_menu.draw(canvas);
        break;
    case State::Playing:
        m_game->draw(canvas);
        break;
    }
}
