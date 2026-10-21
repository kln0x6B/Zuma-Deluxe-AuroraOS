// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef CANVASNODE_H
#define CANVASNODE_H

#include <QHash>
#include <QSGTransformNode>

class Canvas;
class QQuickWindow;
class QSGGeometryNode;
class QSGTexture;
class SpriteMaterial;
class TextureStore;

// Render thread side of a Canvas: owns GPU textures and one geometry node per batch.
class CanvasNode : public QSGTransformNode
{
public:
    explicit CanvasNode(QQuickWindow *window);
    ~CanvasNode() override;

    void syncTextures(const TextureStore &store);
    void syncContent(const Canvas &canvas);

private:
    struct Texture
    {
        QSGTexture *texture;
        SpriteMaterial *material;
    };

    Texture upload(const QString &path) const;
    void release(const Texture &texture);
    QSGGeometryNode *batchNode(int index);

    QQuickWindow *m_window;
    QHash<int, Texture> m_textures;
    QVector<QSGGeometryNode *> m_nodes;
    quint64 m_texturesRevision = 0;
    quint64 m_contentRevision = 0;
};

#endif // CANVASNODE_H
