// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "bitmapfont.h"

#include "texturestore.h"

#include <QFile>

namespace {

// Glyph sheets are authored for 640x480, the game renders at 1.5x of it
const qreal GlyphScale = 1.5;

int valuesPerEntry(const QByteArray &section)
{
    if (section.endsWith("RectList"))
        return 4;
    if (section.endsWith("OffsetList"))
        return 2;
    return 1;
}

quint16 pairKey(char first, char second)
{
    return quint16(quint8(first)) << 8 | quint8(second);
}

}

bool BitmapFont::load(const QString &basePath, TextureStore &textures)
{
    QFile metrics(basePath + QStringLiteral(".txt"));
    if (!metrics.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open font metrics %s", qPrintable(metrics.fileName()));
        return false;
    }

    QHash<QByteArray, QList<QByteArray>> sections;
    const QList<QByteArray> tokens = metrics.readAll().simplified().split(' ');
    for (int i = 0; i + 1 < tokens.size();) {
        const QByteArray &name = tokens.at(i);
        const int count = tokens.at(i + 1).toInt() * valuesPerEntry(name);
        sections.insert(name, tokens.mid(i + 2, count));
        i += 2 + count;
    }

    const QList<QByteArray> chars = sections.value("CharList");
    const QList<QByteArray> widths = sections.value("WidthList");
    const QList<QByteArray> rects = sections.value("RectList");
    const QList<QByteArray> offsets = sections.value("OffsetList");
    if (chars.isEmpty() || widths.size() != chars.size()
            || rects.size() != chars.size() * 4 || offsets.size() != chars.size() * 2) {
        qWarning("Malformed font metrics %s", qPrintable(metrics.fileName()));
        return false;
    }

    m_glyphs.resize(chars.size());
    for (int i = 0; i < chars.size(); ++i) {
        Glyph &glyph = m_glyphs[i];
        glyph.advance = widths.at(i).toInt();
        glyph.rect = QRect(rects.at(i * 4).toInt(), rects.at(i * 4 + 1).toInt(),
                           rects.at(i * 4 + 2).toInt(), rects.at(i * 4 + 3).toInt());
        glyph.offset = QPoint(offsets.at(i * 2).toInt(), offsets.at(i * 2 + 1).toInt());
        m_index.insert(chars.at(i).at(0), i);
    }

    const QList<QByteArray> pairs = sections.value("KerningPairs");
    const QList<QByteArray> values = sections.value("KerningValues");
    for (int i = 0; i < qMin(pairs.size(), values.size()); ++i) {
        if (pairs.at(i).size() == 2)
            m_kerning.insert(pairKey(pairs.at(i).at(0), pairs.at(i).at(1)), values.at(i).toInt());
    }

    m_texture = textures.add(basePath + QStringLiteral(".png"));
    return m_texture >= 0;
}

const BitmapFont::Glyph *BitmapFont::glyph(char character) const
{
    const auto it = m_index.constFind(character);
    return it == m_index.constEnd() ? nullptr : &m_glyphs.at(it.value());
}

int BitmapFont::kerning(char current, char next) const
{
    return m_kerning.value(pairKey(current, next));
}

void BitmapFont::draw(Canvas &canvas, const QByteArray &text, const QPointF &pos,
                      qreal scale, Color color) const
{
    const qreal factor = GlyphScale * scale;
    int x = 0;
    int y = 0;

    for (int i = 0; i < text.size(); ++i) {
        const char character = text.at(i);
        if (character == '\n') {
            x = 0;
            y += m_glyphs.first().rect.height();
        } else if (character == ' ') {
            x += m_glyphs.first().advance;
        }

        const Glyph *g = glyph(character);
        if (!g)
            continue;

        x += kerning(character, i + 1 < text.size() ? text.at(i + 1) : '\0');
        const QRectF target(pos.x() + (x + g->offset.x()) * factor,
                            pos.y() + (y + g->offset.y()) * factor,
                            g->rect.width() * factor, g->rect.height() * factor);
        canvas.draw(m_texture, g->rect, target, color);
        x += g->advance;
    }
}

int BitmapFont::width(const QByteArray &text) const
{
    int widest = 0;
    int line = 0;
    int x = 0;

    for (int i = 0; i < text.size(); ++i) {
        const char character = text.at(i);
        if (character == '\n') {
            widest = qMax(widest, line);
            x = 0;
        } else if (character == ' ') {
            x += m_glyphs.first().advance;
        }

        const Glyph *g = glyph(character);
        if (!g)
            continue;

        x += kerning(character, i + 1 < text.size() ? text.at(i + 1) : '\0');
        x += g->advance;
        line = int(x * GlyphScale);
    }

    return qMax(widest, line);
}
