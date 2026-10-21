// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "gameview.h"

#include "engine/canvas.h"
#include "engine/canvasnode.h"
#include "engine/engine.h"
#include "zuma.h"

#include <QQuickWindow>

#include <auroraapp.h>

namespace {

const qint64 TickNanoseconds = 1000000000 / Engine::TickRate;
const int MaxTicksPerFrame = 4;

}

GameView::GameView(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton);
}

GameView::~GameView() = default;

void GameView::componentComplete()
{
    QQuickItem::componentComplete();

    m_engine.reset(new Engine(Aurora::Application::pathTo(QStringLiteral("data")).toLocalFile()));
    m_canvas.reset(new Canvas(m_engine->textures()));
    m_canvas->setFonts(&m_engine->fonts());
    m_zuma.reset(new Zuma(*m_engine));
    updateViewport();

    connect(m_zuma.get(), &Zuma::quitRequested, this, &GameView::quitRequested);
    connect(m_zuma.get(), &Zuma::statusChanged, this, &GameView::statusChanged);

    if (m_active) {
        m_active = false;
        setActive(true);
    }
}

bool GameView::isRunning() const
{
    return m_active && m_zuma && window();
}

void GameView::setActive(bool active)
{
    if (m_active == active)
        return;
    m_active = active;

    if (m_zuma) {
        m_engine->audio().setActive(active);
        m_zuma->setPaused(!active);
    }
    if (isRunning()) {
        m_clock.start();
        m_lag = 0;
        update();
    }
    emit activeChanged();
}

QString GameView::levelName() const
{
    return m_zuma ? m_zuma->levelName() : QString();
}

int GameView::score() const
{
    return m_zuma ? m_zuma->score() : -1;
}

void GameView::itemChange(ItemChange change, const ItemChangeData &data)
{
    if (change == ItemSceneChange) {
        disconnect(m_frameConnection);
        if (data.window)
            m_frameConnection = connect(data.window, &QQuickWindow::frameSwapped,
                                        this, &GameView::onFrame, Qt::QueuedConnection);
    }
    QQuickItem::itemChange(change, data);
}

void GameView::onFrame()
{
    if (!isRunning())
        return;

    m_lag += m_clock.nsecsElapsed();
    m_clock.restart();

    int ticks = 0;
    while (m_lag >= TickNanoseconds && ticks < MaxTicksPerFrame) {
        tick();
        m_lag -= TickNanoseconds;
        ++ticks;
    }
    // After a stall the game slows down instead of fast-forwarding
    m_lag = qMin(m_lag, TickNanoseconds);

    if (ticks > 0) {
        m_canvas->clear();
        m_zuma->draw(*m_canvas);
    }
    update();
}

void GameView::tick()
{
    Pointer &pointer = m_engine->pointer();
    pointer.pressed = false;
    pointer.released = false;

    // A release arriving in the same tick as its press is kept for the next tick,
    // so that every tap is seen as held at least once.
    if (m_pressPending) {
        pointer.down = true;
        pointer.pressed = true;
        m_pressPending = false;
    } else if (m_releasePending) {
        pointer.down = false;
        pointer.released = true;
        m_releasePending = false;
    }
    pointer.pos = m_pointerPos;

    m_engine->tick();
    m_zuma->update();
}

QSGNode *GameView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_engine || width() <= 0 || height() <= 0) {
        delete oldNode;
        return nullptr;
    }

    CanvasNode *node = static_cast<CanvasNode *>(oldNode);
    if (!node)
        node = new CanvasNode(window());

    node->syncTextures(m_engine->textures());
    node->syncContent(*m_canvas);

    QMatrix4x4 matrix;
    matrix.scale(m_scale);
    matrix.translate(-m_viewport.x(), -m_viewport.y());
    node->setMatrix(matrix);
    return node;
}

void GameView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    updateViewport();
}

// The whole logical screen stays visible; the spare side of the display widens it symmetrically.
void GameView::updateViewport()
{
    if (width() <= 0 || height() <= 0)
        return;

    const QSizeF screen = Engine::ScreenSize;
    m_scale = qMin(width() / screen.width(), height() / screen.height());
    const QSizeF visible(width() / m_scale, height() / m_scale);
    m_viewport = QRectF(QPointF((screen.width() - visible.width()) / 2,
                                (screen.height() - visible.height()) / 2), visible);

    if (m_engine) {
        m_engine->setViewport(m_viewport);
        m_canvas->setViewport(m_viewport);
    }
    update();
}

void GameView::setPointer(const QPointF &pos)
{
    m_pointerPos = pos / m_scale + m_viewport.topLeft();
}

void GameView::mousePressEvent(QMouseEvent *event)
{
    setPointer(event->localPos());
    m_pressPending = true;
    m_releasePending = false;
    m_touching = true;
    setKeepMouseGrab(true);
    event->accept();
}

void GameView::mouseMoveEvent(QMouseEvent *event)
{
    setPointer(event->localPos());
}

void GameView::mouseReleaseEvent(QMouseEvent *event)
{
    setPointer(event->localPos());
    m_releasePending = true;
    m_touching = false;
    setKeepMouseGrab(false);
}

void GameView::mouseUngrabEvent()
{
    if (!m_touching)
        return;

    // An interrupted touch is lifted outside of the screen so that it activates nothing
    m_pointerPos = QPointF(-Engine::ScreenSize.width(), -Engine::ScreenSize.height());
    m_releasePending = true;
    m_touching = false;
}
