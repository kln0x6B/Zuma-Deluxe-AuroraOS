// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef GAMEVIEW_H
#define GAMEVIEW_H

#include <QElapsedTimer>
#include <QQuickItem>

#include <memory>

class Canvas;
class Engine;
class Zuma;

// Hosts the game in a Qt Quick scene: runs fixed 60 Hz ticks in step with the display
// and renders the logical 1280x720 screen scaled to the item size.
class GameView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString levelName READ levelName NOTIFY statusChanged)
    Q_PROPERTY(int score READ score NOTIFY statusChanged)

public:
    explicit GameView(QQuickItem *parent = nullptr);
    ~GameView() override;

    bool isActive() const { return m_active; }
    void setActive(bool active);

    QString levelName() const;
    int score() const;

signals:
    void activeChanged();
    void statusChanged();
    void quitRequested();

protected:
    void componentComplete() override;
    void itemChange(ItemChange change, const ItemChangeData &data) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseUngrabEvent() override;

private:
    void onFrame();
    void tick();
    void setPointer(const QPointF &pos);
    void updateViewport();
    bool isRunning() const;

    std::unique_ptr<Engine> m_engine;
    std::unique_ptr<Canvas> m_canvas;
    std::unique_ptr<Zuma> m_zuma;
    QMetaObject::Connection m_frameConnection;
    QElapsedTimer m_clock;
    qint64 m_lag = 0;
    qreal m_scale = 1;
    QRectF m_viewport;
    bool m_active = false;

    QPointF m_pointerPos;
    bool m_pressPending = false;
    bool m_releasePending = false;
    bool m_touching = false;
};

#endif // GAMEVIEW_H
