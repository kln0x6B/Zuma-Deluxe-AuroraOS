// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef POINTER_H
#define POINTER_H

#include <QPointF>
#include <QRectF>

// Touch/mouse state sampled once per game tick, in logical screen coordinates.
struct Pointer
{
    QPointF pos;
    bool down = false;
    bool pressed = false;
    bool released = false;

    // A finger that has been lifted no longer hovers anything after its release tick.
    bool tracking() const { return down || released; }
    bool over(const QRectF &rect) const { return tracking() && rect.contains(pos); }
};

#endif // POINTER_H
