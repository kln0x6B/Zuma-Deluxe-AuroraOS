// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "dialog.h"

#include "engine/engine.h"
#include "game/assets.h"

namespace {

const int Tile = 121;
const int SlideSpeed = 96;
const QPointF Center(640, 360);
const QRect ButtonSprite(0, 532, 180, 72);
const Color LabelColor(255, 255, 0);

int tileOffset(int index, int count)
{
    return index == 0 ? 0 : (index == count - 1 ? Tile * 2 : Tile);
}

}

QString formatTime(int seconds)
{
    return QStringLiteral("%1:%2").arg(seconds / 60).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

const Color Dialog::StatColor(255, 152, 0);

std::unique_ptr<Dialog> Dialog::options(Audio &audio)
{
    std::unique_ptr<Dialog> dialog(new Dialog(QStringLiteral("OPTIONS"), 5, 4));

    const QPointF music(-56, -128);
    dialog->addSlider(audio.musicVolume(), [&audio](float value) { audio.setMusicVolume(value); }, music);
    dialog->addLabel(QStringLiteral("MUSIC VOLUME"), music + QPointF(-170, 10), Assets::Cancun10, LabelColor);

    const QPointF sound = music + QPointF(0, 64);
    dialog->addSlider(audio.soundVolume(), [&audio](float value) { audio.setSoundVolume(value); }, sound);
    dialog->addLabel(QStringLiteral("SOUNDS VOLUME"), sound + QPointF(-170, 10), Assets::Cancun10, LabelColor);

    dialog->addButton(QStringLiteral("DONE"), 6, QPointF(-236, 120));
    return dialog;
}

std::unique_ptr<Dialog> Dialog::pause(Audio &audio)
{
    std::unique_ptr<Dialog> dialog = options(audio);
    dialog->m_caption = QStringLiteral("mENU");
    dialog->m_buttons[Resume].widget.text = QStringLiteral("BACK TO GAME");
    dialog->addButton(QStringLiteral("RETURN TO MAIN MENU"), 6, QPointF(-236, 48));
    return dialog;
}

std::unique_ptr<Dialog> Dialog::stats(const QString &caption, const QStringList &rows, int leftRows)
{
    std::unique_ptr<Dialog> dialog(new Dialog(caption, 6, 4, true));

    QVector<QPointF> positions;
    for (int i = 0; i < rows.size(); ++i) {
        const bool left = i < leftRows;
        positions.append(QPointF(left ? -262 : 32, -100 + 28 * (left ? i : i - leftRows)));
        dialog->addLabel(rows.at(i), positions.last(), Assets::Cancun12, LabelColor);
    }
    for (const QPointF &position : positions)
        dialog->addLabel(QString(), position + QPointF(185, 0), Assets::Cancun12, StatColor);

    dialog->m_statRows = rows.size();
    dialog->addButton(QStringLiteral("OK"), 8, QPointF(-297, 120));
    dialog->slideIn();
    return dialog;
}

// Authorship and licensing; the game fonts have no lowercase letters
std::unique_ptr<Dialog> Dialog::credits()
{
    std::unique_ptr<Dialog> dialog(new Dialog(QStringLiteral("CREDITS"), 7, 4));

    const struct { const char *text; bool heading; } lines[] = {
        { "ORIGINAL GAME: ZUMA DELUXE HD", true },
        { "BY GALAXYSHAD AND S4LAT", false },
        { "GITHUB.COM/GALAXYSHAD/ZUMA-DELUXE-HD", false },
        { "AURORA OS PORT: KIRILL KOLENO", true },
        { "KLN0X6B@SUPPORT.RU", false },
        { "GITHUB.COM/KLN0X6B/ZUMA-DELUXE-AURORAOS", false },
        { "SOURCE CODE IS DISTRIBUTED UNDER THE MIT LICENSE", true },
        { "GAME ASSETS (C) POPCAP GAMES AND ELECTRONIC ARTS", false },
    };
    int y = -160;
    for (const auto &line : lines) {
        dialog->addLabel(QLatin1String(line.text), QPointF(0, y), Assets::Cancun10,
                         line.heading ? LabelColor : Color(), TextShadow | TextCenter);
        y += 26;
    }

    dialog->addButton(QStringLiteral("OK"), 8, QPointF(-297, 120));
    return dialog;
}

Dialog::Dialog(const QString &caption, int columns, int rows, bool godHead)
    : m_caption(caption)
    , m_columns(columns)
    , m_rows(rows)
    , m_godHead(godHead)
{
}

void Dialog::addLabel(const QString &text, const QPointF &offset, int font, Color color, int flags)
{
    m_labels.append({ text, offset, font, color, flags });
}

void Dialog::addButton(const QString &text, int stretch, const QPointF &offset)
{
    Button button(Assets::MenuTexture, ButtonSprite, origin() + offset);
    button.text = text;
    button.stretch = stretch;
    m_buttons.append({ button, offset });
}

void Dialog::addSlider(float value, std::function<void(float)> onChanged, const QPointF &offset)
{
    Slider slider(value, std::move(onChanged));
    slider.pos = origin() + offset;
    m_sliders.append({ slider, offset });
}

void Dialog::setStat(int row, const QString &text, Color color)
{
    Label &label = m_labels[m_statRows + row];
    label.text = text;
    label.color = color;
}

void Dialog::setStats(const QStringList &values)
{
    for (int row = 0; row < values.size(); ++row)
        setStat(row, values.at(row));
}

void Dialog::slideIn()
{
    m_slide = int(Engine::ScreenSize.height()) + Tile * m_rows / 2;
    layout();
}

QPointF Dialog::origin() const
{
    return Center + QPointF(0, m_slide);
}

void Dialog::layout()
{
    for (Placed<Button> &button : m_buttons)
        button.widget.pos = origin() + button.offset;
    for (Placed<Slider> &slider : m_sliders)
        slider.widget.pos = origin() + slider.offset;
}

void Dialog::update(Engine &engine)
{
    if (m_slide > 0) {
        m_slide -= qMin(SlideSpeed, m_slide);
        layout();
        return;
    }

    for (int i = 0; i < m_buttons.size(); ++i) {
        m_buttons[i].widget.update(engine);
        if (m_buttons.at(i).widget.clicked())
            m_clicked = i;
    }
    for (Placed<Slider> &slider : m_sliders)
        slider.widget.update(engine);
}

int Dialog::takeClicked()
{
    const int clicked = m_clicked;
    m_clicked = -1;
    return clicked;
}

void Dialog::draw(Canvas &canvas) const
{
    const QPointF center = origin();
    const QPointF topLeft = center - QPointF(Tile * m_columns / 2, Tile * m_rows / 2);

    for (int row = 0; row < m_rows; ++row) {
        for (int column = 0; column < m_columns; ++column) {
            const QRect source(tileOffset(column, m_columns), tileOffset(row, m_rows), Tile, Tile);
            canvas.draw(Assets::MenuTexture, source,
                        QRectF(topLeft + QPointF(Tile * column, Tile * row), QSizeF(Tile, Tile)));
        }
    }

    if (m_godHead)
        canvas.draw(Assets::MenuTexture, QRect(363, 0, 234, 210),
                    QRectF(center.x() - 117, topLeft.y() - 112, 234, 210));

    canvas.text(Assets::NativeAlienExt18, m_caption,
                QPointF(center.x(), topLeft.y() + (m_godHead ? 72 : 32)), Color(), TextCenter);

    for (const Placed<Button> &button : m_buttons)
        button.widget.draw(canvas);
    for (const Placed<Slider> &slider : m_sliders)
        slider.widget.draw(canvas);
    for (const Label &label : m_labels)
        canvas.text(label.font, label.text, center + label.offset, label.color, label.flags);
}
