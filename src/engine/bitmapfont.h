// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef BITMAPFONT_H
#define BITMAPFONT_H

#include "canvas.h"

#include <QHash>
#include <QPoint>
#include <QRect>
#include <QVector>

class TextureStore;

// Bitmap font described by a PopCap .txt metrics file and a glyph sheet .png.
class BitmapFont
{
public:
    bool load(const QString &basePath, TextureStore &textures);

    void draw(Canvas &canvas, const QByteArray &text, const QPointF &pos, qreal scale, Color color) const;
    int width(const QByteArray &text) const;

private:
    struct Glyph
    {
        QRect rect;
        QPoint offset;
        int advance = 0;
    };

    const Glyph *glyph(char character) const;
    int kerning(char current, char next) const;

    int m_texture = -1;
    QVector<Glyph> m_glyphs;
    QHash<char, int> m_index;
    QHash<quint16, int> m_kerning;
};

#endif // BITMAPFONT_H
