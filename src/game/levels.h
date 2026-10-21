// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef LEVELS_H
#define LEVELS_H

#include <QHash>
#include <QPointF>
#include <QString>
#include <QVector>

class Canvas;
class Engine;

// Level data is authored for 640x480; the playfield is centered on a 1280x720 screen.
inline QPointF toScreen(const QPointF &point)
{
    return QPointF((point.x() + 104) * 1.5, point.y() * 1.5);
}

struct LevelSettings
{
    QString id;
    float speed = 0.5f;
    int startCount = 35;
    int gaugeScore = 1000;
    int repeatChance = 40;
    int singleChance = 6;
    int colors = 4;
    int parTime = 30;
    float slowFactor = 4;
};

struct LevelInfo
{
    QString id;
    QString curve;
    QString image;
    QString imageTop;
    QString displayName;
    QPointF frog;
    QVector<QPointF> treasures;
};

class LevelLibrary
{
public:
    bool load(const QString &path);

    int count() const { return m_stage.size(); }
    const LevelInfo &level(int index) const { return m_levels.at(m_stage.at(index)); }
    LevelSettings settings(const QString &id) const { return m_settings.value(id); }

private:
    QVector<LevelInfo> m_levels;
    QHash<QString, LevelSettings> m_settings;
    QVector<int> m_stage;
};

// The curve balls roll along, in screen coordinates.
class LevelPath
{
public:
    struct Step
    {
        QPointF delta;
        qreal angle;
        bool tunnel;
        bool priority;
    };

    bool load(const QString &path);

    int length() const { return m_steps.size(); }
    const Step &step(int index) const;
    QPointF point(int index) const;
    QPointF at(qreal position) const;
    QPointF start() const { return m_points.first(); }
    QPointF end() const { return m_points.last(); }

private:
    QVector<Step> m_steps;
    QVector<QPointF> m_points;
};

class Level
{
public:
    Level(Engine &engine, const LevelInfo &info);
    ~Level();

    bool isValid() const;
    const LevelInfo &info() const { return m_info; }
    const LevelPath &path() const { return m_path; }

    void drawBackground(Canvas &canvas) const;
    void drawForeground(Canvas &canvas) const;

private:
    void drawLayer(Canvas &canvas, int texture) const;

    Engine &m_engine;
    const LevelInfo &m_info;
    LevelPath m_path;
    int m_background = -1;
    int m_foreground = -1;
};

#endif // LEVELS_H
