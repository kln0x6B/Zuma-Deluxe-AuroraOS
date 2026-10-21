// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef CANVAS_H
#define CANVAS_H

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

class BitmapFont;
class TextureStore;

struct Color
{
    constexpr Color(quint8 red = 255, quint8 green = 255, quint8 blue = 255, quint8 alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}

    quint8 r, g, b, a;
};

enum TextFlag {
    TextPlain = 0,
    TextShadow = 1 << 0,
    TextCenter = 1 << 1,
};

// Records textured quads in drawing order; consecutive quads sharing a texture form one batch.
class Canvas
{
public:
    struct Vertex
    {
        float x, y;
        float u, v;
        quint8 r, g, b, a;
    };

    struct Batch
    {
        int texture;
        int firstQuad;
        int quadCount;
    };

    explicit Canvas(const TextureStore &textures);

    void setFonts(const QVector<BitmapFont> *fonts) { m_fonts = fonts; }

    // Visible logical area: the 1280x720 screen widened to the display aspect ratio.
    void setViewport(const QRectF &viewport) { m_viewport = viewport; }
    const QRectF &viewport() const { return m_viewport; }

    void clear();

    void draw(int texture, const QRectF &source, const QRectF &target,
              Color color = Color(), qreal angle = 0);
    void drawCentered(int texture, const QRectF &source, const QPointF &center,
                      qreal scale = 1, qreal angle = 0, Color color = Color());
    void drawImage(int texture, const QPointF &center);
    // Fills the screen with a background and continues it into the viewport margins mirrored.
    void drawBackdrop(int texture, const QRectF &source, const QRectF &target);

    void text(int font, const QString &text, const QPointF &pos,
              Color color = Color(), int flags = TextPlain, qreal scale = 1);
    int textWidth(int font, const QString &text) const;

    const QVector<Vertex> &vertices() const { return m_vertices; }
    const QVector<Batch> &batches() const { return m_batches; }
    quint64 revision() const { return m_revision; }

private:
    const TextureStore &m_textures;
    const QVector<BitmapFont> *m_fonts = nullptr;
    QRectF m_viewport;
    QVector<Vertex> m_vertices;
    QVector<Batch> m_batches;
    quint64 m_revision = 0;
};

#endif // CANVAS_H
