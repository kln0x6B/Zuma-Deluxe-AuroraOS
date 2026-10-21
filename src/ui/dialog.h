// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#ifndef DIALOG_H
#define DIALOG_H

#include "engine/canvas.h"
#include "widgets.h"

#include <QStringList>
#include <QVector>

#include <memory>

class Audio;

QString formatTime(int seconds);

// Stone panel assembled from 121px tiles, optionally crowned with the god head.
class Dialog
{
public:
    enum OptionsButton { Done };
    enum PauseButton { Resume, MainMenu };
    enum StatsButton { Confirm };
    enum CreditsButton { Close };

    static std::unique_ptr<Dialog> options(Audio &audio);
    static std::unique_ptr<Dialog> pause(Audio &audio);
    static std::unique_ptr<Dialog> stats(const QString &caption, const QStringList &rows, int leftRows);
    static std::unique_ptr<Dialog> credits();

    Dialog(const QString &caption, int columns, int rows, bool godHead = false);

    void addLabel(const QString &text, const QPointF &offset, int font, Color color, int flags = TextShadow);
    void addButton(const QString &text, int stretch, const QPointF &offset);
    void addSlider(float value, std::function<void(float)> onChanged, const QPointF &offset);

    // Updates the value shown next to a row of a stats dialog.
    void setStat(int row, const QString &text, Color color = StatColor);
    void setStats(const QStringList &values);

    void slideIn();
    void update(Engine &engine);
    int takeClicked();
    void draw(Canvas &canvas) const;

private:
    static const Color StatColor;

    template<typename Widget>
    struct Placed
    {
        Widget widget;
        QPointF offset;
    };

    struct Label
    {
        QString text;
        QPointF offset;
        int font;
        Color color;
        int flags;
    };

    QPointF origin() const;
    void layout();

    QString m_caption;
    int m_columns;
    int m_rows;
    bool m_godHead;
    int m_slide = 0;
    int m_clicked = -1;
    int m_statRows = 0;
    QVector<Label> m_labels;
    QVector<Placed<Button>> m_buttons;
    QVector<Placed<Slider>> m_sliders;
};

#endif // DIALOG_H
