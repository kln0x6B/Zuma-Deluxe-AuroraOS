// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#include "audio.h"

#include <QAudioOutput>
#include <QtMath>

namespace {

const int OutputRate = 44100;
const int OutputChannels = 2;
const int FrameBytes = OutputChannels * int(sizeof(qint16));
const int BufferFrames = OutputRate / 12;
const int MaxVoices = 32;

}

class Audio::Mixer : public QIODevice
{
public:
    explicit Mixer(Audio &audio) : m_audio(audio) {}

    bool isSequential() const override { return true; }
    qint64 bytesAvailable() const override { return BufferFrames * FrameBytes + QIODevice::bytesAvailable(); }

protected:
    qint64 readData(char *data, qint64 maxSize) override
    {
        const int frames = int(maxSize / FrameBytes);
        m_audio.mix(reinterpret_cast<qint16 *>(data), frames);
        return qint64(frames) * FrameBytes;
    }

    qint64 writeData(const char *, qint64) override { return -1; }

private:
    Audio &m_audio;
};

Audio::Audio(QObject *parent)
    : QObject(parent)
    , m_mixer(new Mixer(*this))
{
    QAudioFormat format;
    format.setSampleRate(OutputRate);
    format.setChannelCount(OutputChannels);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setCodec(QStringLiteral("audio/pcm"));

    m_output = new QAudioOutput(format, this);
    m_output->setBufferSize(BufferFrames * FrameBytes);
    m_mixer->open(QIODevice::ReadOnly);
}

Audio::~Audio()
{
    m_output->stop();
    delete m_mixer;
}

int Audio::loadSample(const QString &path)
{
    PcmBuffer buffer;
    if (!OggStream::decode(path, buffer) || buffer.channels > OutputChannels)
        return -1;

    m_samples.append(buffer);
    return m_samples.size() - 1;
}

void Audio::play(int sample)
{
    startVoice(sample, 1.0, false);
}

void Audio::playPitched(int sample, float semitones)
{
    for (int i = m_voices.size() - 1; i >= 0; --i) {
        if (m_voices.at(i).sample == sample && m_voices.at(i).pitched)
            m_voices.remove(i);
    }
    startVoice(sample, qPow(2.0, semitones / 12.0), true);
}

void Audio::stop(int sample)
{
    for (int i = m_voices.size() - 1; i >= 0; --i) {
        if (m_voices.at(i).sample == sample)
            m_voices.remove(i);
    }
}

void Audio::startVoice(int sample, double pitch, bool pitched)
{
    if (sample < 0 || sample >= m_samples.size())
        return;

    if (m_voices.size() >= MaxVoices)
        m_voices.removeFirst();

    const double step = double(m_samples.at(sample).rate) / OutputRate * pitch;
    m_voices.append({ sample, 0.0, step, m_soundVolume, pitched });
}

void Audio::playMusic(const QString &path)
{
    if (!m_music.open(path))
        return;

    if (m_music.channels() != OutputChannels || m_music.rate() != OutputRate) {
        qWarning("Music %s must be %d Hz stereo", qPrintable(path), OutputRate);
        m_music.close();
    }
}

void Audio::stopMusic()
{
    m_music.close();
}

void Audio::setMusicVolume(float volume)
{
    m_musicVolume = qBound(0.0f, volume, 1.0f);
}

void Audio::setSoundVolume(float volume)
{
    m_soundVolume = qBound(0.0f, volume, 1.0f);
}

void Audio::setActive(bool active)
{
    if (m_active == active)
        return;

    m_active = active;

    // The stream is recreated on every activation: resume() of the PulseAudio backend
    // may leave a stream corked after the application returns to the foreground.
    if (active) {
        if (!m_mixer->isOpen())
            m_mixer->open(QIODevice::ReadOnly);
        m_output->start(m_mixer);
    } else {
        m_output->stop();
    }
}

void Audio::mix(qint16 *out, int frames)
{
    m_accumulator.fill(0.0f, frames * OutputChannels);
    float *accumulator = m_accumulator.data();

    for (int i = m_voices.size() - 1; i >= 0; --i) {
        Voice &voice = m_voices[i];
        const PcmBuffer &pcm = m_samples.at(voice.sample);
        const qint16 *data = pcm.samples.constData();
        const int last = pcm.frames() - 1;

        int frame = 0;
        for (; frame < frames; ++frame) {
            const int index = int(voice.position);
            if (index >= last)
                break;

            const float fraction = float(voice.position - index);
            for (int channel = 0; channel < OutputChannels; ++channel) {
                const int source = pcm.channels == 1 ? index : index * 2 + channel;
                const float current = data[source];
                const float next = data[source + pcm.channels];
                accumulator[frame * OutputChannels + channel]
                        += (current + (next - current) * fraction) * voice.gain;
            }
            voice.position += voice.step;
        }

        if (frame < frames)
            m_voices.remove(i);
    }

    mixMusic(accumulator, frames);

    for (int i = 0; i < frames * OutputChannels; ++i)
        out[i] = qint16(qBound(-32768.0f, accumulator[i], 32767.0f));
}

void Audio::mixMusic(float *accumulator, int frames)
{
    if (!m_music.isOpen() || m_musicVolume <= 0.0f)
        return;

    m_musicBuffer.resize(frames * OutputChannels);
    int done = 0;
    bool rewound = false;
    while (done < frames) {
        const int read = m_music.read(m_musicBuffer.data() + done * OutputChannels, frames - done);
        if (read > 0) {
            done += read;
            rewound = false;
        } else if (!rewound) {
            m_music.rewind();
            rewound = true;
        } else {
            break;
        }
    }

    for (int i = 0; i < done * OutputChannels; ++i)
        accumulator[i] += m_musicBuffer.at(i) * m_musicVolume;
}
