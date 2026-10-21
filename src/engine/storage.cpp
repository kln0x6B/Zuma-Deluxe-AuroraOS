// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "storage.h"

#include <QStandardPaths>

namespace {

QString settingsPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)
            + QStringLiteral("/settings.ini");
}

QString recordKey(const QString &level, int difficulty, const char *field)
{
    return QStringLiteral("records/%1/%2/%3").arg(level).arg(difficulty).arg(QLatin1String(field));
}

}

Storage::Storage()
    : m_settings(settingsPath(), QSettings::IniFormat)
{
}

float Storage::musicVolume() const
{
    return m_settings.value(QStringLiteral("audio/music"), 1.0).toFloat();
}

float Storage::soundVolume() const
{
    return m_settings.value(QStringLiteral("audio/sound"), 1.0).toFloat();
}

void Storage::setVolumes(float music, float sound)
{
    m_settings.setValue(QStringLiteral("audio/music"), music);
    m_settings.setValue(QStringLiteral("audio/sound"), sound);
    m_settings.sync();
}

Storage::Record Storage::record(const QString &level, int difficulty) const
{
    Record record;
    record.score = m_settings.value(recordKey(level, difficulty, "score")).toInt();
    record.time = m_settings.value(recordKey(level, difficulty, "time")).toInt();
    return record;
}

void Storage::setRecord(const QString &level, int difficulty, const Record &record)
{
    m_settings.setValue(recordKey(level, difficulty, "score"), record.score);
    m_settings.setValue(recordKey(level, difficulty, "time"), record.time);
    m_settings.sync();
}
