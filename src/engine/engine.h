// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef ENGINE_H
#define ENGINE_H

#include "audio.h"
#include "bitmapfont.h"
#include "pointer.h"
#include "storage.h"
#include "texturestore.h"

#include <QRectF>

// Shared services of the game: resources, audio, input, persistent data and game time.
class Engine
{
public:
    static const int TickRate = 60;
    static const QSizeF ScreenSize;
    static QRectF screenRect() { return QRectF(QPointF(), ScreenSize); }

    explicit Engine(const QString &dataDir);

    QString path(const QString &relative) const;

    TextureStore &textures() { return m_textures; }
    const TextureStore &textures() const { return m_textures; }
    const QVector<BitmapFont> &fonts() const { return m_fonts; }
    Audio &audio() { return m_audio; }
    Storage &storage() { return m_storage; }
    Pointer &pointer() { return m_pointer; }
    const Pointer &pointer() const { return m_pointer; }

    void saveAudioSettings();

    int loadTexture(const QString &relative);
    int loadFont(const QString &relative);
    int loadSound(const QString &relative);

    // The logical screen widened to the display aspect ratio, see Canvas::viewport()
    const QRectF &viewport() const { return m_viewport; }
    void setViewport(const QRectF &viewport) { m_viewport = viewport; }

    void tick() { ++m_ticks; }
    double time() const { return double(m_ticks) / TickRate; }

    // Inclusive range, like the original game's randInt()
    static int random(int min, int max);

private:
    QString m_dataDir;
    TextureStore m_textures;
    QVector<BitmapFont> m_fonts;
    Audio m_audio;
    Storage m_storage;
    Pointer m_pointer;
    QRectF m_viewport = screenRect();
    quint64 m_ticks = 0;
};

class Canvas;

class Scene
{
public:
    virtual ~Scene() = default;

    virtual void update() = 0;
    virtual void draw(Canvas &canvas) const = 0;
};

#endif // ENGINE_H
