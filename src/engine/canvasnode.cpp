// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "canvasnode.h"

#include "canvas.h"
#include "texturestore.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QQuickWindow>
#include <QSGGeometryNode>
#include <QSGMaterial>
#include <QSGTexture>

namespace {

class SpriteShader : public QSGMaterialShader
{
public:
    void updateState(const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *) override;
    char const *const *attributeNames() const override;

protected:
    void initialize() override;
    const char *vertexShader() const override;
    const char *fragmentShader() const override;

private:
    int m_matrix = -1;
    int m_opacity = -1;
};

}

// Multiplies a premultiplied texture by a premultiplied per-vertex color.
class SpriteMaterial : public QSGMaterial
{
public:
    explicit SpriteMaterial(QSGTexture *texture) : m_texture(texture) { setFlag(Blending); }

    QSGTexture *texture() const { return m_texture; }

    QSGMaterialType *type() const override
    {
        static QSGMaterialType type;
        return &type;
    }

    QSGMaterialShader *createShader() const override { return new SpriteShader; }

    int compare(const QSGMaterial *other) const override
    {
        return m_texture->textureId() - static_cast<const SpriteMaterial *>(other)->m_texture->textureId();
    }

private:
    QSGTexture *m_texture;
};

namespace {

void SpriteShader::initialize()
{
    m_matrix = program()->uniformLocation("matrix");
    m_opacity = program()->uniformLocation("opacity");
}

void SpriteShader::updateState(const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *)
{
    if (state.isMatrixDirty())
        program()->setUniformValue(m_matrix, state.combinedMatrix());
    if (state.isOpacityDirty())
        program()->setUniformValue(m_opacity, state.opacity());
    static_cast<SpriteMaterial *>(newMaterial)->texture()->bind();
}

char const *const *SpriteShader::attributeNames() const
{
    static const char *const names[] = { "vertex", "texCoord", "color", nullptr };
    return names;
}

const char *SpriteShader::vertexShader() const
{
    return "uniform highp mat4 matrix;\n"
           "uniform lowp float opacity;\n"
           "attribute highp vec4 vertex;\n"
           "attribute highp vec2 texCoord;\n"
           "attribute lowp vec4 color;\n"
           "varying highp vec2 vTexCoord;\n"
           "varying lowp vec4 vColor;\n"
           "void main() {\n"
           "    vTexCoord = texCoord;\n"
           "    vColor = color * opacity;\n"
           "    gl_Position = matrix * vertex;\n"
           "}\n";
}

const char *SpriteShader::fragmentShader() const
{
    return "uniform lowp sampler2D source;\n"
           "varying highp vec2 vTexCoord;\n"
           "varying lowp vec4 vColor;\n"
           "void main() {\n"
           "    gl_FragColor = texture2D(source, vTexCoord) * vColor;\n"
           "}\n";
}

const QSGGeometry::AttributeSet &spriteAttributes()
{
    static const QSGGeometry::Attribute attributes[] = {
        QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
        QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
        QSGGeometry::Attribute::create(2, 4, GL_UNSIGNED_BYTE),
    };
    static const QSGGeometry::AttributeSet set = { 3, int(sizeof(Canvas::Vertex)), attributes };
    return set;
}

int maxTextureSize()
{
    GLint size = 2048;
    QOpenGLContext::currentContext()->functions()->glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
    return size;
}

}

CanvasNode::CanvasNode(QQuickWindow *window)
    : m_window(window)
{
}

CanvasNode::~CanvasNode()
{
    for (const Texture &texture : m_textures)
        release(texture);
}

void CanvasNode::syncTextures(const TextureStore &store)
{
    if (store.revision() == m_texturesRevision)
        return;
    m_texturesRevision = store.revision();
    m_contentRevision = 0;

    for (auto it = m_textures.begin(); it != m_textures.end();) {
        if (store.entries().contains(it.key())) {
            ++it;
        } else {
            release(it.value());
            it = m_textures.erase(it);
        }
    }

    const QHash<int, TextureStore::Entry> &entries = store.entries();
    for (auto it = entries.constBegin(); it != entries.constEnd(); ++it) {
        if (m_textures.contains(it.key()))
            continue;
        const Texture texture = upload(it.value().path);
        if (texture.texture)
            m_textures.insert(it.key(), texture);
    }
}

CanvasNode::Texture CanvasNode::upload(const QString &path) const
{
    static const int limit = maxTextureSize();

    QImage image(path);
    if (image.isNull()) {
        qWarning("Unable to load texture %s", qPrintable(path));
        return { nullptr, nullptr };
    }
    // Texture coordinates are normalized, so a downscaled copy maps transparently
    if (image.width() > limit || image.height() > limit)
        image = image.scaled(limit, limit, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    QSGTexture *texture = m_window->createTextureFromImage(image);
    texture->setFiltering(QSGTexture::Linear);
    texture->setHorizontalWrapMode(QSGTexture::ClampToEdge);
    texture->setVerticalWrapMode(QSGTexture::ClampToEdge);
    return { texture, new SpriteMaterial(texture) };
}

void CanvasNode::release(const Texture &texture)
{
    delete texture.material;
    delete texture.texture;
}

// A new node is returned detached: the renderer requires geometry and material before it joins the tree.
QSGGeometryNode *CanvasNode::batchNode(int index)
{
    if (index < m_nodes.size())
        return m_nodes.at(index);

    QSGGeometry *geometry = new QSGGeometry(spriteAttributes(), 0, 0, GL_UNSIGNED_SHORT);
    geometry->setDrawingMode(GL_TRIANGLES);
    geometry->setVertexDataPattern(QSGGeometry::StreamPattern);
    geometry->setIndexDataPattern(QSGGeometry::StreamPattern);

    QSGGeometryNode *node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    return node;
}

void CanvasNode::syncContent(const Canvas &canvas)
{
    if (canvas.revision() == m_contentRevision)
        return;
    m_contentRevision = canvas.revision();

    const Canvas::Vertex *vertices = canvas.vertices().constData();
    int used = 0;

    for (const Canvas::Batch &batch : canvas.batches()) {
        const auto texture = m_textures.constFind(batch.texture);
        if (texture == m_textures.constEnd())
            continue;

        QSGGeometryNode *node = batchNode(used++);
        QSGGeometry *geometry = node->geometry();
        geometry->allocate(batch.quadCount * 4, batch.quadCount * 6);
        memcpy(geometry->vertexData(), vertices + batch.firstQuad * 4,
               size_t(batch.quadCount) * 4 * sizeof(Canvas::Vertex));

        quint16 *indices = geometry->indexDataAsUShort();
        for (int quad = 0; quad < batch.quadCount; ++quad) {
            const quint16 base = quint16(quad * 4);
            const quint16 corners[] = { base, quint16(base + 1), quint16(base + 2),
                                        base, quint16(base + 2), quint16(base + 3) };
            memcpy(indices + quad * 6, corners, sizeof(corners));
        }

        node->setMaterial(texture->material);
        if (node->parent()) {
            node->markDirty(QSGNode::DirtyGeometry | QSGNode::DirtyMaterial);
        } else {
            appendChildNode(node);
            m_nodes.append(node);
        }
    }

    while (m_nodes.size() > used) {
        QSGGeometryNode *node = m_nodes.takeLast();
        removeChildNode(node);
        delete node;
    }
}
