// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef TEXTURESTORE_H
#define TEXTURESTORE_H

#include <QHash>
#include <QSize>
#include <QString>

class TextureStore
{
public:
    struct Entry
    {
        QString path;
        QSize size;
    };

    int add(const QString &path);
    void remove(int handle);

    QSize size(int handle) const;
    const QHash<int, Entry> &entries() const { return m_entries; }
    quint64 revision() const { return m_revision; }

private:
    QHash<int, Entry> m_entries;
    int m_nextHandle = 0;
    quint64 m_revision = 0;
};

#endif // TEXTURESTORE_H
