// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "menuscreen.h"

#include "engine/engine.h"
#include "game/assets.h"
#include "game/levels.h"

namespace {

enum MainButton { Adventure, Gauntlet, Options, MoreGames, Quit };
enum GauntletButton { BackToMain, PreviousLevel, Play, NextLevel, FirstDifficulty };

const int DifficultyCount = 4;
const Color White;
const Color Yellow(255, 255, 0);

struct ButtonSpec
{
    QRect sprite;
    QPointF pos;
    float hoverPitch;
};

const ButtonSpec MainButtons[] = {
    { QRect(2724, 901, 245, 138), QPointF(840, 95), 12 },
    { QRect(2648, 776, 270, 125), QPointF(816, 230), 10 },
    { QRect(2563, 647, 299, 129), QPointF(790, 354), 8 },
    { QRect(2924, 1038, 177, 190), QPointF(750, 452), 4 },
    { QRect(2917, 1226, 180, 211), QPointF(902, 470), -2 },
};

const ButtonSpec GauntletButtons[] = {
    { QRect(1280, 2160, 211, 84), QPointF(172, 632), 0 },
    { QRect(1924, 2260, 139, 48), QPointF(426, 667), 0 },
    { QRect(1922, 2210, 157, 49), QPointF(576, 667), 0 },
    { QRect(1922, 2161, 141, 48), QPointF(745, 667), 0 },
    { QRect(1608, 2336, 328, 63), QPointF(470, 270), 0 },
    { QRect(1538, 2399, 258, 49), QPointF(504, 196), 0 },
    { QRect(1485, 2448, 205, 43), QPointF(530, 124), 0 },
    { QRect(1439, 2491, 159, 54), QPointF(554, 45), 0 },
};

}

MenuScreen::MenuScreen(Engine &engine, const LevelLibrary &levels)
    : m_engine(engine)
    , m_levels(levels)
{
}

MenuScreen::~MenuScreen() = default;

void MenuScreen::setRoom(Room room, const QRect &background, const QRect &sky)
{
    m_room = room;
    m_background = background;
    m_sky = sky;
    m_buttons.clear();
    m_dialog.reset();
}

void MenuScreen::showMain()
{
    setRoom(Room::Main, QRect(1282, 720, 1280, 720), QRect(0, 720, 1280, 250));
    for (const ButtonSpec &spec : MainButtons) {
        Button button(Assets::MenuTexture, spec.sprite, spec.pos);
        button.deadBorder = 16;
        button.hoverSound = Assets::ChoralSound;
        button.hoverPitch = spec.hoverPitch;
        m_buttons.append(button);
    }
    Assets::playMusic(m_engine, Assets::Music::Menu);
}

void MenuScreen::showGauntlet()
{
    setRoom(Room::Gauntlet, QRect(1282, 1440, 1280, 720), QRect(0, 1442, 1280, 250));
    for (const ButtonSpec &spec : GauntletButtons)
        m_buttons.append(Button(Assets::MenuTexture, spec.sprite, spec.pos));
    m_buttons[FirstDifficulty + m_difficulty].selected = true;
    Assets::playMusic(m_engine, Assets::Music::Gauntlet);
}

MenuScreen::Action MenuScreen::update()
{
    m_skyOffset = (m_skyOffset + 1) % int(Engine::ScreenSize.width());

    if (m_dialog) {
        m_dialog->update(m_engine);
        if (m_dialog->takeClicked() >= 0) {
            m_engine.saveAudioSettings();
            m_dialog.reset();
        }
        return Action::None;
    }

    int clicked = -1;
    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].update(m_engine);
        if (m_buttons.at(i).clicked())
            clicked = i;
    }
    if (clicked < 0)
        return Action::None;

    return m_room == Room::Main ? handleMain(clicked) : handleGauntlet(clicked);
}

MenuScreen::Action MenuScreen::handleMain(int button)
{
    switch (button) {
    case Adventure:
    case Gauntlet:
        showGauntlet();
        return Action::None;
    case Options:
        m_dialog = Dialog::options(m_engine.audio());
        return Action::None;
    case MoreGames:
        m_dialog = Dialog::credits();
        return Action::None;
    case Quit:
        return Action::Quit;
    }
    return Action::None;
}

MenuScreen::Action MenuScreen::handleGauntlet(int button)
{
    const int levels = m_levels.count();
    switch (button) {
    case BackToMain:
        showMain();
        return Action::None;
    case PreviousLevel:
        m_level = (m_level + levels - 1) % levels;
        return Action::None;
    case NextLevel:
        m_level = (m_level + 1) % levels;
        return Action::None;
    case Play:
        m_engine.audio().stopMusic();
        return Action::Play;
    }

    m_difficulty = qBound(0, button - FirstDifficulty, DifficultyCount - 1);
    for (int i = 0; i < DifficultyCount; ++i)
        m_buttons[FirstDifficulty + i].selected = i == m_difficulty;
    return Action::None;
}

void MenuScreen::draw(Canvas &canvas) const
{
    if (m_room == Room::Gauntlet)
        canvas.drawImage(Assets::thumbnail(m_level), QPointF(656, 500));

    const QRectF &viewport = canvas.viewport();
    const int skyStep = m_sky.width() - 1;
    int skyX = m_skyOffset;
    while (skyX > viewport.left())
        skyX -= skyStep;
    for (; skyX < viewport.right(); skyX += skyStep)
        canvas.draw(Assets::MenuTexture, m_sky,
                    QRectF(skyX, viewport.top(), m_sky.width(), m_sky.height() - viewport.top()));
    canvas.drawBackdrop(Assets::MenuTexture, m_background, Engine::screenRect());

    for (const Button &button : m_buttons)
        button.draw(canvas);

    if (m_room == Room::Main)
        drawMainDecor(canvas);
    else
        drawRecords(canvas);

    if (m_dialog)
        m_dialog->draw(canvas);
}

void MenuScreen::drawMainDecor(Canvas &canvas) const
{
    canvas.draw(Assets::MenuTexture, QRect(2563, 315, 130, 138), QRectF(158, 16, 130, 138));
    canvas.draw(Assets::MenuTexture, QRect(2563, 0, 315, 315), QRectF(68, -68, 315, 315), Color(255, 192, 0, 152));

    canvas.text(Assets::NativeAlienExt16, QStringLiteral("WELCOME TO ZUMA HD!"), QPointF(426, 0));
    canvas.text(Assets::Cancun10, QStringLiteral("REMASTER BY\nGALAXYSHAD AND S4LAT. 2021"), QPointF(16, 660));
    canvas.text(Assets::Cancun10, QStringLiteral("ORIGINAL GAME BY POPCAP. 2003"), QPointF(16, 694));
}

void MenuScreen::drawRecords(Canvas &canvas) const
{
    const Storage::Record record = m_engine.storage().record(m_levels.level(m_level).id, m_difficulty);

    canvas.text(Assets::Cancun10, QStringLiteral("BEST SCORE:"), QPointF(924, 632), White, TextShadow);
    canvas.text(Assets::Cancun10, QString::number(record.score), QPointF(1140, 632), Yellow, TextShadow);
    canvas.text(Assets::Cancun10, QStringLiteral("BEST TIME:"), QPointF(924, 686), White, TextShadow);
    canvas.text(Assets::Cancun10, formatTime(record.time), QPointF(1140, 686), Yellow, TextShadow);
}
