// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT

#ifndef AUDIO_H
#define AUDIO_H

#include "oggstream.h"

#include <QIODevice>
#include <QObject>

class QAudioOutput;

// Software mixer for sound effects and a looping music stream.
class Audio : public QObject
{
    Q_OBJECT

public:
    explicit Audio(QObject *parent = nullptr);
    ~Audio() override;

    int loadSample(const QString &path);

    // Starts a new voice; several voices of one sample may overlap.
    void play(int sample);
    // Restarts the only voice of the sample, shifted by the given number of semitones.
    void playPitched(int sample, float semitones);
    void stop(int sample);

    void playMusic(const QString &path);
    void stopMusic();

    float musicVolume() const { return m_musicVolume; }
    float soundVolume() const { return m_soundVolume; }
    void setMusicVolume(float volume);
    void setSoundVolume(float volume);

    void setActive(bool active);

private:
    class Mixer;

    struct Voice
    {
        int sample;
        double position;
        double step;
        float gain;
        bool pitched;
    };

    void startVoice(int sample, double pitch, bool pitched);
    void mix(qint16 *out, int frames);
    void mixMusic(float *accumulator, int frames);

    QVector<PcmBuffer> m_samples;
    QVector<Voice> m_voices;
    QVector<float> m_accumulator;
    QVector<qint16> m_musicBuffer;
    OggStream m_music;
    float m_musicVolume = 1.0f;
    float m_soundVolume = 1.0f;

    Mixer *m_mixer = nullptr;
    QAudioOutput *m_output = nullptr;
    bool m_active = false;
};

#endif // AUDIO_H
