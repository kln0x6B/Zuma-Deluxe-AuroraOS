// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef OGGSTREAM_H
#define OGGSTREAM_H

#include <QString>
#include <QVector>

#include <memory>

struct OggVorbis_File;

struct PcmBuffer
{
    QVector<qint16> samples;
    int channels = 0;
    int rate = 0;

    int frames() const { return channels ? samples.size() / channels : 0; }
};

// Decodes Ogg Vorbis files into interleaved signed 16-bit PCM.
class OggStream
{
public:
    OggStream();
    ~OggStream();

    bool open(const QString &path);
    void close();
    bool isOpen() const { return bool(m_file); }

    int channels() const { return m_channels; }
    int rate() const { return m_rate; }

    // Returns the number of frames read, zero at the end of the stream.
    int read(qint16 *out, int frames);
    void rewind();

    static bool decode(const QString &path, PcmBuffer &buffer);

private:
    struct Closer { void operator()(OggVorbis_File *file) const; };

    std::unique_ptr<OggVorbis_File, Closer> m_file;
    int m_channels = 0;
    int m_rate = 0;
};

#endif // OGGSTREAM_H
