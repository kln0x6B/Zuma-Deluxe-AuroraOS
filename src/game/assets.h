// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef ASSETS_H
#define ASSETS_H

#include <QString>

class Engine;

namespace Assets {

enum Texture {
    FrogTexture,
    ObjectsTexture,
    HudTexture,
    MenuTexture,
    DisclaimerTexture,
    FirstThumbnailTexture,
};

enum Font {
    Cancun10,
    Cancun12,
    CancunFloat14,
    NativeAlien48,
    NativeAlienExt16,
    NativeAlienExt18,
};

enum Sound {
    BallClickSound,
    BallInsertSound,
    MatchSound1,
    MatchSound2,
    MatchSound3,
    MatchSound4,
    MatchSound5,
    ButtonSound,
    SwapSound,
    LevelStartSound,
    LevelCompleteSound,
    GaugeFullSound,
    GameOverSound,
    ChimeSound,
    ChoralSound,
    CoinGrabSound,
    BonusPopSound,
    ShootSound,
    GemVanishSound,
    GemAppearSound,
    LightTrailSound,
    RollingSound,
};

enum class Music {
    Game,
    Menu,
    Gauntlet,
    Win,
    GameOver,
};

const int ThumbnailCount = 18;

bool load(Engine &engine);
void playMusic(Engine &engine, Music music);

inline int thumbnail(int level) { return FirstThumbnailTexture + level; }

}

#endif // ASSETS_H
