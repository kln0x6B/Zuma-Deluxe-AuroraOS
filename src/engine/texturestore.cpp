// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "texturestore.h"

#include <QImageReader>

int TextureStore::add(const QString &path)
{
    const QSize size = QImageReader(path).size();
    if (!size.isValid()) {
        qWarning("Unable to read image %s", qPrintable(path));
        return -1;
    }

    const int handle = m_nextHandle++;
    m_entries.insert(handle, { path, size });
    ++m_revision;
    return handle;
}

void TextureStore::remove(int handle)
{
    if (m_entries.remove(handle))
        ++m_revision;
}

QSize TextureStore::size(int handle) const
{
    return m_entries.value(handle).size;
}
