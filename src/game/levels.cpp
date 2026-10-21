// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "levels.h"

#include "engine/canvas.h"
#include "engine/engine.h"

#include <QDataStream>
#include <QFile>
#include <QStringList>
#include <QXmlStreamReader>
#include <QtMath>

namespace {

LevelInfo parseGraphics(const QXmlStreamAttributes &attributes)
{
    LevelInfo info;
    info.id = attributes.value(QLatin1String("id")).toString();
    info.curve = attributes.value(QLatin1String("curve")).toString();
    info.image = attributes.value(QLatin1String("image")).toString();
    info.imageTop = attributes.value(QLatin1String("image-top")).toString();
    info.displayName = attributes.value(QLatin1String("dispname")).toString();
    info.frog = QPointF(attributes.value(QLatin1String("gx")).toDouble(),
                        attributes.value(QLatin1String("gy")).toDouble());
    return info;
}

LevelSettings parseSettings(const QXmlStreamAttributes &attributes)
{
    auto number = [&](const char *name, double fallback) {
        const QStringRef value = attributes.value(QLatin1String(name));
        return value.isEmpty() ? fallback : value.toDouble();
    };

    LevelSettings settings;
    settings.id = attributes.value(QLatin1String("id")).toString();
    settings.speed = float(number("speed", settings.speed));
    settings.startCount = int(number("start", settings.startCount));
    settings.gaugeScore = int(number("score", settings.gaugeScore));
    settings.repeatChance = int(number("repeat", settings.repeatChance));
    settings.singleChance = int(number("single", settings.singleChance));
    settings.colors = int(number("colors", settings.colors));
    settings.parTime = int(number("partime", settings.parTime));
    settings.slowFactor = float(number("slowfactor", settings.slowFactor));
    return settings;
}

}

bool LevelLibrary::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open %s", qPrintable(path));
        return false;
    }

    QStringList stage;
    bool inGraphics = false;
    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement() && xml.name() == QLatin1String("Graphics"))
            inGraphics = false;
        if (!xml.isStartElement())
            continue;

        const QStringRef name = xml.name();
        const QXmlStreamAttributes attributes = xml.attributes();
        if (name == QLatin1String("Graphics")) {
            m_levels.append(parseGraphics(attributes));
            inGraphics = true;
        } else if (inGraphics && name == QLatin1String("TreasurePoint")) {
            m_levels.last().treasures.append(QPointF(attributes.value(QLatin1String("x")).toDouble(),
                                                     attributes.value(QLatin1String("y")).toDouble()));
        } else if (name == QLatin1String("Settings")) {
            const LevelSettings settings = parseSettings(attributes);
            m_settings.insert(settings.id, settings);
        } else if (name == QLatin1String("StageProgression")) {
            stage = attributes.value(QLatin1String("stage1")).toString().split(QLatin1Char(','));
        }
    }

    if (xml.hasError()) {
        qWarning("Malformed %s: %s", qPrintable(path), qPrintable(xml.errorString()));
        return false;
    }

    for (const QString &id : stage) {
        for (int i = 0; i < m_levels.size(); ++i) {
            if (m_levels.at(i).id == id.trimmed()) {
                m_stage.append(i);
                break;
            }
        }
    }
    return !m_stage.isEmpty();
}

bool LevelPath::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Unable to open %s", qPrintable(path));
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

    qint32 controlPoints = 0;
    file.seek(0x10);
    stream >> controlPoints;
    file.seek(0x14 + qint64(controlPoints) * 10);

    qint32 total = 0;
    float x = 0;
    float y = 0;
    stream >> total >> x >> y;

    m_steps.clear();
    m_points = { toScreen(QPointF(x, y)) };
    for (int i = 0; i + 1 < total && stream.status() == QDataStream::Ok; ++i) {
        quint8 tunnel = 0;
        quint8 priority = 0;
        qint8 dx = 0;
        qint8 dy = 0;
        stream >> tunnel >> priority >> dx >> dy;

        const QPointF delta(dx / 100.0 * 1.5, dy / 100.0 * 1.5);
        m_steps.append({ delta, qRadiansToDegrees(qAtan2(dy, dx)), tunnel != 0, priority != 0 });
        m_points.append(m_points.last() + delta);
    }

    if (stream.status() != QDataStream::Ok || m_steps.isEmpty()) {
        qWarning("Malformed curve %s", qPrintable(path));
        return false;
    }
    return true;
}

const LevelPath::Step &LevelPath::step(int index) const
{
    return m_steps.at(qBound(0, index, m_steps.size() - 1));
}

QPointF LevelPath::point(int index) const
{
    return m_points.at(qBound(0, index, m_points.size() - 1));
}

QPointF LevelPath::at(qreal position) const
{
    const int index = qBound(0, int(position), length() - 1);
    return point(index) + (point(index + 1) - point(index)) * (position - index);
}

Level::Level(Engine &engine, const LevelInfo &info)
    : m_engine(engine)
    , m_info(info)
{
    const QString directory = QStringLiteral("levels/%1/").arg(info.id);
    m_path.load(engine.path(directory + info.curve + QStringLiteral(".dat")));
    m_background = engine.loadTexture(directory + info.image + QStringLiteral(".jpg"));
    if (!info.imageTop.isEmpty())
        m_foreground = engine.loadTexture(directory + info.imageTop + QStringLiteral(".png"));
}

Level::~Level()
{
    m_engine.textures().remove(m_background);
    m_engine.textures().remove(m_foreground);
}

bool Level::isValid() const
{
    return m_background >= 0 && m_path.length() > 0;
}

void Level::drawBackground(Canvas &canvas) const
{
    drawLayer(canvas, m_background);
}

void Level::drawForeground(Canvas &canvas) const
{
    drawLayer(canvas, m_foreground);
}

void Level::drawLayer(Canvas &canvas, int texture) const
{
    if (texture >= 0)
        canvas.drawBackdrop(texture, QRectF(QPointF(), m_engine.textures().size(texture)), Engine::screenRect());
}
