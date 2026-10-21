// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "engine.h"

#include <QDateTime>
#include <QDir>

const QSizeF Engine::ScreenSize(1280, 720);

Engine::Engine(const QString &dataDir)
    : m_dataDir(dataDir)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch()));
    m_audio.setMusicVolume(m_storage.musicVolume());
    m_audio.setSoundVolume(m_storage.soundVolume());
}

QString Engine::path(const QString &relative) const
{
    return QDir(m_dataDir).filePath(relative);
}

void Engine::saveAudioSettings()
{
    m_storage.setVolumes(m_audio.musicVolume(), m_audio.soundVolume());
}

int Engine::loadTexture(const QString &relative)
{
    return m_textures.add(path(relative));
}

int Engine::loadFont(const QString &relative)
{
    BitmapFont font;
    if (!font.load(path(relative), m_textures))
        return -1;
    m_fonts.append(font);
    return m_fonts.size() - 1;
}

int Engine::loadSound(const QString &relative)
{
    return m_audio.loadSample(path(relative));
}

int Engine::random(int min, int max)
{
    return min + qrand() % (max - min + 1);
}
