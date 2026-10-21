// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "assets.h"

#include "engine/engine.h"

#include <QStringList>

namespace Assets {

namespace {

const char *const Images[] = {
    "images/frog.png",
    "images/gameobjects.png",
    "images/gameinterface.png",
    "images/menu.png",
    "images/disclaimer.jpg",
};

const char *const Fonts[] = {
    "fonts/cancun10",
    "fonts/cancun12",
    "fonts/cancunfloat14",
    "fonts/nativealien48",
    "fonts/nativealienextended16",
    "fonts/nativealienextended18",
};

const char *const Sounds[] = {
    "ballclick1",
    "ballclick2",
    "ballsdestroyed1",
    "ballsdestroyed2",
    "ballsdestroyed3",
    "ballsdestroyed4",
    "ballsdestroyed5",
    "button1",
    "button2",
    "chant1",
    "chant2",
    "chant4",
    "chant8",
    "chime1",
    "choral1",
    "coingrab",
    "endoflevelpop1",
    "fireball1",
    "gemvanishes",
    "jewelappear",
    "lighttrail2",
    "rolling",
};

const char *const Tracks[] = {
    "game",
    "menu",
    "gauntlet",
    "win",
    "gameover",
};

// Resources are addressed by enum values, so each handle must match its list position.
template<typename Load>
bool loadAll(const QStringList &paths, Load load)
{
    for (int i = 0; i < paths.size(); ++i) {
        if (load(paths.at(i)) != i) {
            qWarning("Unable to load %s", qPrintable(paths.at(i)));
            return false;
        }
    }
    return true;
}

}

bool load(Engine &engine)
{
    QStringList textures;
    for (const char *path : Images)
        textures << QLatin1String(path);
    for (int level = 1; level <= ThumbnailCount; ++level)
        textures << QStringLiteral("images/thumbnails/%1.jpg").arg(level, 2, 10, QLatin1Char('0'));

    QStringList fonts;
    for (const char *path : Fonts)
        fonts << QLatin1String(path);

    QStringList sounds;
    for (const char *name : Sounds)
        sounds << QStringLiteral("sounds/%1.ogg").arg(QLatin1String(name));

    return loadAll(textures, [&](const QString &path) { return engine.loadTexture(path); })
            && loadAll(fonts, [&](const QString &path) { return engine.loadFont(path); })
            && loadAll(sounds, [&](const QString &path) { return engine.loadSound(path); });
}

void playMusic(Engine &engine, Music music)
{
    const QLatin1String track(Tracks[int(music)]);
    engine.audio().playMusic(engine.path(QStringLiteral("music/%1.ogg").arg(track)));
}

}
