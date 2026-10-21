// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "oggstream.h"

#include <QFile>

#include <vorbis/vorbisfile.h>

void OggStream::Closer::operator()(OggVorbis_File *file) const
{
    ov_clear(file);
    delete file;
}

OggStream::OggStream() = default;

OggStream::~OggStream() = default;

bool OggStream::open(const QString &path)
{
    close();

    std::unique_ptr<OggVorbis_File> file(new OggVorbis_File);
    if (ov_fopen(QFile::encodeName(path).constData(), file.get()) != 0) {
        qWarning("Unable to open Ogg Vorbis file %s", qPrintable(path));
        return false;
    }

    const vorbis_info *info = ov_info(file.get(), -1);
    m_channels = info->channels;
    m_rate = int(info->rate);
    m_file.reset(file.release());
    return true;
}

void OggStream::close()
{
    m_file.reset();
    m_channels = 0;
    m_rate = 0;
}

int OggStream::read(qint16 *out, int frames)
{
    if (!m_file)
        return 0;

    const int frameBytes = m_channels * int(sizeof(qint16));
    char *data = reinterpret_cast<char *>(out);
    int remaining = frames * frameBytes;
    int bitstream = 0;

    while (remaining > 0) {
        const long bytes = ov_read(m_file.get(), data, remaining, 0, 2, 1, &bitstream);
        if (bytes <= 0)
            break;
        data += bytes;
        remaining -= int(bytes);
    }

    return frames - remaining / frameBytes;
}

void OggStream::rewind()
{
    if (m_file)
        ov_pcm_seek(m_file.get(), 0);
}

bool OggStream::decode(const QString &path, PcmBuffer &buffer)
{
    OggStream stream;
    if (!stream.open(path))
        return false;

    buffer.channels = stream.channels();
    buffer.rate = stream.rate();
    buffer.samples.resize(int(ov_pcm_total(stream.m_file.get(), -1)) * buffer.channels);
    buffer.samples.resize(stream.read(buffer.samples.data(), buffer.frames()) * buffer.channels);
    return !buffer.samples.isEmpty();
}
