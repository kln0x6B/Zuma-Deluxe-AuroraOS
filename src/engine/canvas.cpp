// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "canvas.h"

#include "bitmapfont.h"
#include "texturestore.h"

#include <QtMath>

namespace {

// Keeps every batch addressable with 16-bit indices
const int MaxQuadsPerBatch = 0xFFFF / 4;

const Color ShadowColor(0, 0, 0, 192);

}

Canvas::Canvas(const TextureStore &textures)
    : m_textures(textures)
{
}

void Canvas::clear()
{
    m_vertices.clear();
    m_batches.clear();
    ++m_revision;
}

void Canvas::draw(int texture, const QRectF &source, const QRectF &target, Color color, qreal angle)
{
    const QSize textureSize = m_textures.size(texture);
    if (textureSize.isEmpty() || color.a == 0)
        return;

    if (m_batches.isEmpty() || m_batches.last().texture != texture
            || m_batches.last().quadCount == MaxQuadsPerBatch)
        m_batches.append({ texture, m_vertices.size() / 4, 0 });
    ++m_batches.last().quadCount;

    const QPointF center = target.center();
    const qreal halfWidth = target.width() / 2;
    const qreal halfHeight = target.height() / 2;
    const qreal radians = qDegreesToRadians(angle);
    const qreal cosine = qCos(radians);
    const qreal sine = qSin(radians);

    const float left = source.left() / textureSize.width();
    const float right = source.right() / textureSize.width();
    const float top = source.top() / textureSize.height();
    const float bottom = source.bottom() / textureSize.height();

    const quint8 red = color.r * color.a / 255;
    const quint8 green = color.g * color.a / 255;
    const quint8 blue = color.b * color.a / 255;

    auto corner = [&](qreal dx, qreal dy, float u, float v) {
        m_vertices.append({ float(center.x() + dx * cosine - dy * sine),
                            float(center.y() + dx * sine + dy * cosine),
                            u, v, red, green, blue, color.a });
    };
    corner(-halfWidth, -halfHeight, left, top);
    corner(halfWidth, -halfHeight, right, top);
    corner(halfWidth, halfHeight, right, bottom);
    corner(-halfWidth, halfHeight, left, bottom);
}

void Canvas::drawCentered(int texture, const QRectF &source, const QPointF &center,
                          qreal scale, qreal angle, Color color)
{
    const QSizeF size = source.size() * scale;
    draw(texture, source, QRectF(center - QPointF(size.width() / 2, size.height() / 2), size),
         color, angle);
}

void Canvas::drawImage(int texture, const QPointF &center)
{
    drawCentered(texture, QRectF(QPointF(), m_textures.size(texture)), center);
}

void Canvas::drawBackdrop(int texture, const QRectF &source, const QRectF &target)
{
    draw(texture, source, target);

    const qreal scaleX = source.width() / target.width();
    const qreal scaleY = source.height() / target.height();
    const qreal left = target.left() - m_viewport.left();
    const qreal right = m_viewport.right() - target.right();
    const qreal top = target.top() - m_viewport.top();
    const qreal bottom = m_viewport.bottom() - target.bottom();

    // Source rectangles of negative size flip the strip, so it mirrors the edge it touches
    if (left > 0)
        draw(texture, QRectF(source.left() + left * scaleX, source.top(), -left * scaleX, source.height()),
             QRectF(m_viewport.left(), target.top(), left, target.height()));
    if (right > 0)
        draw(texture, QRectF(source.right(), source.top(), -right * scaleX, source.height()),
             QRectF(target.right(), target.top(), right, target.height()));
    if (top > 0)
        draw(texture, QRectF(source.left(), source.top() + top * scaleY, source.width(), -top * scaleY),
             QRectF(target.left(), m_viewport.top(), target.width(), top));
    if (bottom > 0)
        draw(texture, QRectF(source.left(), source.bottom(), source.width(), -bottom * scaleY),
             QRectF(target.left(), target.bottom(), target.width(), bottom));
}

void Canvas::text(int font, const QString &text, const QPointF &pos, Color color, int flags, qreal scale)
{
    if (!m_fonts || font < 0 || font >= m_fonts->size())
        return;

    const BitmapFont &bitmapFont = m_fonts->at(font);
    const QByteArray latin = text.toLatin1();

    QPointF origin = pos;
    if (flags & TextCenter)
        origin.rx() -= bitmapFont.width(latin) / 2 * scale;

    if (flags & TextShadow)
        bitmapFont.draw(*this, latin, origin + QPointF(1, -1), scale, ShadowColor);
    bitmapFont.draw(*this, latin, origin, scale, color);
}

int Canvas::textWidth(int font, const QString &text) const
{
    if (!m_fonts || font < 0 || font >= m_fonts->size())
        return 0;
    return m_fonts->at(font).width(text.toLatin1());
}
