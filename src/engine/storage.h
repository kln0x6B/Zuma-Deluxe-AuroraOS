// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef STORAGE_H
#define STORAGE_H

#include <QSettings>

// Persistent settings and records, kept in the application configuration directory.
class Storage
{
public:
    struct Record
    {
        int score = 0;
        int time = 0;
    };

    Storage();

    float musicVolume() const;
    float soundVolume() const;
    void setVolumes(float music, float sound);

    Record record(const QString &level, int difficulty) const;
    void setRecord(const QString &level, int difficulty, const Record &record);

private:
    QSettings m_settings;
};

#endif // STORAGE_H
