// SPDX-FileCopyrightText: 2023 GalaxyShad
// SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
// SPDX-License-Identifier: MIT
// Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD

#include "game.h"

#include "assets.h"
#include "engine/engine.h"

namespace {

const float SlowdownZone = 0.8f;
const float PitOpenZone = 0.2f;
const int PitFrames = 12;
const int GaugeWidth = 96;

const float SparkleSpeed = 15;
const float SparkleLastFrame = 13;
const float BurstSpeed = 12;
const float BurstLastFrame = 16;
const float BurstSpacing = 50;
const float ParticleFrameSpeed = 0.5f;
const double LightTrailInterval = 0.3;
const int OutroBonus = 100;

const double TreasureLifetime = 10;
const double TreasureBlinkTime = 3;
const int TreasureBlinkRate = 8;
const qreal TreasureHitDistanceSquared = 2900;
const float TreasureFadeSpeed = 0.1f;
const float TreasureMaxScale = 2.5f;
const float TreasureSpinSpeed = 0.25f;
const int TreasureBonus = 500;

const Color Yellow(255, 255, 0);
const Color Green(0, 255, 0);

// The gauntlet uses one settings preset per difficulty, tuned on top of the level progression.
struct Difficulty
{
    const char *settings;
    int colors;
    int parTime;
    int startCount;
    int repeatChance;
};

const Difficulty Difficulties[] = {
    { "level5", 4, 70, 40, -1 },
    { "level9", 5, 100, 50, -1 },
    { "level14", 6, 120, 60, 25 },
    { "level25", 6, 150, 60, -1 },
};

LevelSettings difficultySettings(const LevelLibrary &levels, int difficulty, int bestTime)
{
    const Difficulty &preset = Difficulties[qBound(0, difficulty, 3)];
    LevelSettings settings = levels.settings(QLatin1String(preset.settings));
    settings.colors = preset.colors;
    settings.parTime = bestTime > 0 ? bestTime : preset.parTime;
    settings.startCount = preset.startCount;
    if (preset.repeatChance >= 0)
        settings.repeatChance = preset.repeatChance;
    return settings;
}

int wrapLevel(int index, int count)
{
    return (index % count + count) % count;
}

}

Game::Game(Engine &engine, const LevelLibrary &levels, int levelIndex, int difficulty)
    : m_engine(engine)
    , m_levelIndex(wrapLevel(levelIndex, levels.count()))
    , m_difficulty(difficulty)
    , m_level(new Level(engine, levels.level(m_levelIndex)))
    , m_settings(difficultySettings(levels, difficulty,
                                    engine.storage().record(m_level->info().id, difficulty).time))
    , m_frog(toScreen(m_level->info().frog + QPointF(2, 0)))
    , m_chain(engine, m_level->path(), m_settings)
    , m_pit(Assets::ObjectsTexture, QRect(629, 132, 132, 132))
    , m_menuButton(Assets::HudTexture, QRect(925, 15, 118, 37), QPointF(1128, 4))
    , m_gauge(773, 28, 0, 28)
{
    m_pit.setRange(0, PitFrames, 0);

    m_treasure.since = engine.time();
    m_treasure.sprite = Sprite(Assets::ObjectsTexture, QRect(340, 0, 54, 54));
    m_treasure.sprite.setRange(0, 30, TreasureSpinSpeed);

    engine.audio().play(Assets::LevelStartSound);
    Assets::playMusic(engine, Assets::Music::Game);
}

Game::~Game() = default;

void Game::pause()
{
    if (m_introEnded && !m_won && !m_lost && !m_menu)
        m_menu = Dialog::pause(m_engine.audio());
}

void Game::update()
{
    step();
    animate();
}

bool Game::hasDialog() const
{
    return m_menu || m_stats || m_gameOver;
}

bool Game::playfieldVisible() const
{
    return m_introEnded && !(m_won && !m_outroEnded) && !hasDialog();
}

void Game::step()
{
    m_frog.aim(m_engine.pointer().pos);

    if (!m_introEnded) {
        updateIntro();
        return;
    }

    const bool shot = takeShot();

    if (updateEnding() || updatePauseMenu())
        return;

    m_chain.update(m_score, m_texts);
    if (!m_chain.isEmpty() && m_bullets.update(m_engine.viewport()))
        m_chain.chainBonus = 0;

    if (m_chain.generating && m_score > m_settings.gaugeScore) {
        m_chain.generating = false;
        m_chain.startGlow(m_engine.time());
        m_engine.audio().play(Assets::GaugeFullSound);
    }

    generateChain();
    checkWin();
    if (checkLoss())
        return;

    m_menuButton.update(m_engine);
    if (m_menuButton.clicked())
        m_menu = Dialog::pause(m_engine.audio());

    if (shot && !m_frog.isRecoiling() && m_menuButton.state() != Button::Pressed)
        shoot();

    keepFrogColorsInChain();
    updateChainSpeed();
    m_bullets.collide(m_chain, m_level->path());
    m_texts.update(m_engine.time());
    updateTreasure();
}

// A tap on the frog swaps its balls, a tap anywhere else shoots toward it.
bool Game::takeShot()
{
    const Pointer &pointer = m_engine.pointer();
    if (!pointer.released || hasDialog() || m_menuButton.rect().contains(pointer.pos)
            || !m_engine.viewport().contains(pointer.pos))
        return false;

    if (m_frog.contains(pointer.pos)) {
        m_frog.swapColors();
        m_engine.audio().play(Assets::SwapSound);
        return false;
    }

    return !m_chain.endReached && !m_firstWave && !m_won && !m_lost && !m_chain.isEmpty();
}

void Game::shoot()
{
    m_bullets.fire(m_frog);
    m_engine.audio().play(Assets::ShootSound);
    m_frog.markShot();
    m_frog.color = m_frog.nextColor;
    m_frog.nextColor = randomChainColor();
}

int Game::randomChainColor() const
{
    int color = Engine::random(0, m_chain.colors() - 1);
    while (!m_chain.isEmpty() && !m_chain.hasColor(color))
        color = Engine::random(0, m_chain.colors() - 1);
    return color;
}

void Game::keepFrogColorsInChain()
{
    if (m_lost || m_won || m_chain.endReached || m_chain.isEmpty())
        return;

    if (!m_chain.hasColor(m_frog.nextColor))
        m_frog.nextColor = randomChainColor();
    if (!m_chain.hasColor(m_frog.color))
        m_frog.color = randomChainColor();
}

void Game::updateChainSpeed()
{
    m_chain.speed = m_settings.speed;
    if (m_chain.isEmpty())
        return;

    if (m_chain.ball(m_chain.lastMovingBall()).pos > m_level->path().length() * SlowdownZone)
        m_chain.speed = m_chain.endReached ? BallChain::RollingToPitSpeed
                                           : m_settings.speed / m_settings.slowFactor;
}

// The first wave pours out quickly; afterwards balls follow at regular spacing.
void Game::generateChain()
{
    if (!m_chain.generating)
        return;

    if (m_chain.isEmpty()) {
        m_chain.append();
        return;
    }

    const int tail = m_chain.size() - 1;
    if (m_firstWave && m_chain.size() < m_settings.startCount) {
        if (m_chain.ball(tail).pos >= 0.5f) {
            m_chain.append();
            m_chain.ball(tail + 1).speed = m_chain.ball(tail).speed;
        }
        return;
    }

    if (m_chain.ball(tail).pos >= BallChain::Pad) {
        m_chain.append();
        Ball &ball = m_chain.ball(tail + 1);
        ball.speed = m_chain.ball(tail).speed;
        ball.fadingIn = true;
        ball.resetSprite();
    }

    if (m_firstWave)
        m_engine.audio().stop(Assets::RollingSound);
    m_firstWave = false;
}

void Game::updateIntro()
{
    const LevelPath &path = m_level->path();

    if (!m_sweep.started) {
        Sprite sparkle(Assets::ObjectsTexture, QRect(732, 2253, 30, 30));
        sparkle.color.b = 0;
        sparkle.scale = 1.5f;
        m_sweep = Sweep();
        m_sweep.particles = Particles(sparkle);
        m_sweep.head = path.start();
        m_sweep.started = true;
        m_sweep.particles.spawn(m_sweep.head);
        m_lightTrailTime = m_engine.time();
        return;
    }

    m_sweep.particles.update(SparkleLastFrame, ParticleFrameSpeed);

    const float progress = m_sweep.progress / path.length();
    if (progress > 0.8f && m_headerScale > 0)
        m_headerScale -= 0.1f;
    else if (progress > 0.1f && m_headerScale < 1)
        m_headerScale += 0.1f;
    if (progress > 0.75f && m_subheaderScale > 0)
        m_subheaderScale -= 0.1f;
    else if (progress > 0.15f && m_subheaderScale < 1)
        m_subheaderScale += 0.1f;

    if (m_sweep.progress > path.length()) {
        if (m_sweep.particles.isActive())
            return;
        m_engine.audio().play(Assets::RollingSound);
        m_introEnded = true;
        m_sweep = Sweep();
        m_startTime = m_engine.time();
        return;
    }

    if (m_sweep.progress > m_sweep.lastSpawn) {
        m_sweep.particles.spawn(m_sweep.head);
        m_sweep.lastSpawn = m_sweep.progress;
    }
    advanceSweep(SparkleSpeed);

    if (m_engine.time() - m_lightTrailTime > LightTrailInterval) {
        m_engine.audio().playPitched(Assets::LightTrailSound, m_lightTrailPitch);
        m_lightTrailTime = m_engine.time();
        m_lightTrailPitch -= 1;
    }
}

void Game::updateOutro()
{
    if (!m_sweep.started) {
        Sprite burst(Assets::ObjectsTexture, QRect(528, 0, 100, 130));
        burst.scale = 1.5f;
        m_sweep = Sweep();
        m_sweep.particles = Particles(burst);
        m_sweep.head = m_headPoint;
        m_sweep.progress = m_sweep.lastSpawn = m_headPos;
        m_sweep.started = true;
        awardOutroBonus();
        return;
    }

    m_texts.update(m_engine.time());
    m_sweep.particles.update(BurstLastFrame, ParticleFrameSpeed);

    if (m_sweep.progress > m_level->path().length()) {
        if (m_sweep.particles.isActive())
            return;
        m_engine.audio().play(Assets::LevelCompleteSound);
        m_outroEnded = true;
        m_sweep = Sweep();
        m_stats->setStat(0, QString::number(m_score));
        return;
    }

    if (m_sweep.progress - m_sweep.lastSpawn > BurstSpacing) {
        awardOutroBonus();
        m_sweep.lastSpawn = m_sweep.progress;
    }
    advanceSweep(BurstSpeed);
}

void Game::advanceSweep(float speed)
{
    m_sweep.head += m_level->path().step(int(m_sweep.progress)).delta * speed;
    m_sweep.progress += speed;
}

void Game::awardOutroBonus()
{
    m_score += OutroBonus;
    m_engine.audio().play(Assets::BonusPopSound);
    m_texts.spawn(QStringLiteral("+%1").arg(OutroBonus), m_sweep.head, 2, m_engine.time());
    m_sweep.particles.spawn(m_sweep.head);
}

bool Game::updateEnding()
{
    if (!m_won && !m_lost)
        return false;

    if (m_won && !m_outroEnded) {
        updateOutro();
        return false;
    }

    Dialog *dialog = m_won ? m_stats.get() : m_gameOver.get();
    dialog->update(m_engine);
    if (dialog->takeClicked() == Dialog::Confirm) {
        if (m_won)
            saveRecord();
        m_finished = true;
    }
    return true;
}

bool Game::updatePauseMenu()
{
    if (!m_menu)
        return false;

    m_menu->update(m_engine);
    switch (m_menu->takeClicked()) {
    case Dialog::Resume:
        m_engine.saveAudioSettings();
        m_menu.reset();
        break;
    case Dialog::MainMenu:
        m_engine.saveAudioSettings();
        m_finished = true;
        break;
    }
    return true;
}

void Game::checkWin()
{
    if (m_chain.endReached || !m_chain.isEmpty() || m_score <= m_settings.gaugeScore || m_won)
        return;

    finishLevel(Assets::Music::Win);
    m_stats = Dialog::stats(QStringLiteral("STATS"),
                            QStringList(QStringLiteral("POINTS")) + statNames()
                            + QStringList { QStringLiteral("YOUR TIME"), QStringLiteral("ACE TIME") }, 5);
    m_stats->setStats(QStringList(QString::number(m_score)) + statValues()
                      + QStringList { formatTime(m_elapsed), formatTime(m_settings.parTime) });
    if (m_elapsed <= m_settings.parTime)
        m_stats->setStat(5, formatTime(m_elapsed), Green);
    m_won = true;
}

bool Game::checkLoss()
{
    if (!m_chain.isEmpty() || !m_chain.endReached)
        return false;
    if (m_lost)
        return true;

    m_engine.audio().play(Assets::GameOverSound);
    finishLevel(Assets::Music::GameOver);
    m_gameOver = Dialog::stats(QStringLiteral("GAmE OvER"), QStringList(QStringLiteral("TIME")) + statNames(), 3);
    m_gameOver->setStats(QStringList(formatTime(m_elapsed)) + statValues());
    m_lost = true;
    return false;
}

void Game::saveRecord()
{
    Storage &storage = m_engine.storage();
    Storage::Record record = storage.record(m_level->info().id, m_difficulty);
    record.score = qMax(record.score, m_score);
    if (record.time == 0 || m_elapsed < record.time)
        record.time = m_elapsed;
    storage.setRecord(m_level->info().id, m_difficulty, record);
}

void Game::finishLevel(Assets::Music music)
{
    Assets::playMusic(m_engine, music);
    m_elapsed = int(m_engine.time() - m_startTime);
}

// Rows shared by the results and the game over dialogs
QStringList Game::statNames()
{
    return { QStringLiteral("COMBOS"), QStringLiteral("COINS"),
             QStringLiteral("MAX CHAIN"), QStringLiteral("MAX COMBO") };
}

QStringList Game::statValues() const
{
    const BallChain::Stats &stats = m_chain.stats();
    return { QString::number(stats.totalCombos), QString::number(m_coins),
             QString::number(stats.maxChainBonus), QString::number(stats.maxCombo) };
}

QPointF Game::treasurePosition() const
{
    return toScreen(m_level->info().treasures.at(m_treasure.point));
}

void Game::updateTreasure()
{
    const QVector<QPointF> &points = m_level->info().treasures;
    if (points.isEmpty())
        return;

    const double now = m_engine.time();
    const double age = now - m_treasure.since;

    if (m_treasure.active && m_treasure.blinking) {
        if (age > TreasureBlinkTime) {
            m_treasure.active = false;
            m_treasure.blinking = false;
            m_treasure.since = now;
            m_engine.audio().play(Assets::GemVanishSound);
        }
    } else if (m_treasure.active) {
        if (age > TreasureLifetime) {
            m_treasure.blinking = true;
            m_treasure.since = now;
        }
    } else {
        if (age > TreasureLifetime) {
            m_engine.audio().play(Assets::GemAppearSound);
            m_treasure.active = true;
            m_treasure.point = Engine::random(0, points.size() - 1);
            m_treasure.since = now;
        }
        if (m_treasure.fading && m_treasure.sprite.scale > TreasureMaxScale) {
            m_treasure.fading = false;
            m_treasure.sprite.color.a = 255;
            m_treasure.sprite.scale = 1;
            m_treasure.sprite.speed = TreasureSpinSpeed;
        }
    }

    if (!m_treasure.active)
        return;

    QVector<Bullets::Bullet> &bullets = m_bullets.items();
    for (int i = bullets.size() - 1; i >= 0; --i) {
        const QPointF delta = bullets.at(i).pos - treasurePosition();
        if (QPointF::dotProduct(delta, delta) >= TreasureHitDistanceSquared)
            continue;

        m_chain.chainBonus = 0;
        m_treasure.active = false;
        m_treasure.blinking = false;
        m_treasure.fading = true;
        m_treasure.sprite.speed = 1;
        m_treasure.since = now;
        bullets.remove(i);
        m_score += TreasureBonus;
        ++m_coins;
        m_texts.spawn(QStringLiteral("BONUS +%1").arg(TreasureBonus), treasurePosition(), 2, now);
        m_engine.audio().play(Assets::CoinGrabSound);
        break;
    }
}

bool Game::treasureVisible() const
{
    if (m_level->info().treasures.isEmpty())
        return false;
    if (m_treasure.fading)
        return true;
    if (!m_treasure.active)
        return false;
    return !m_treasure.blinking
            || int((m_engine.time() - m_treasure.since) * TreasureBlinkRate) % 2 == 0;
}

void Game::animateTreasure()
{
    Sprite &sprite = m_treasure.sprite;
    if (m_treasure.fading) {
        sprite.scale += TreasureFadeSpeed;
        sprite.color.a = quint8(qMax(0.0f, 255 * (1 - sprite.scale / TreasureMaxScale)));
    }

    // The original advanced the animation once for the shadow and once for the gem
    if (treasureVisible()) {
        sprite.advance();
        sprite.advance();
    }
}

void Game::animate()
{
    m_frog.animate(m_introEnded);

    if (m_score < m_settings.gaugeScore) {
        const int target = GaugeWidth * m_score / m_settings.gaugeScore;
        if (m_gauge.width() < target)
            m_gauge.setWidth(m_gauge.width() + 1);
    } else {
        m_gauge = QRect(m_gauge.x(), 0, GaugeWidth, m_gauge.height());
    }

    if (!m_chain.isEmpty()) {
        const Ball &head = m_chain.ball(0);
        m_headPos = head.pos;
        m_headPoint = head.point;

        const float length = m_level->path().length();
        const float distance = length - head.pos;
        if (distance < length * PitOpenZone)
            m_pit.frame = int(PitFrames * ((length * PitOpenZone - distance) / (length * PitOpenZone)));
    }

    if (playfieldVisible()) {
        m_chain.animate();
        animateTreasure();
    }
}

void Game::draw(Canvas &canvas) const
{
    m_level->drawBackground(canvas);
    drawPit(canvas);
    m_frog.draw(canvas, m_introEnded);

    if (!m_introEnded) {
        drawHud(canvas);
        drawIntro(canvas);
        return;
    }

    if (m_won && !m_outroEnded) {
        drawHud(canvas);
        m_sweep.particles.draw(canvas);
        m_texts.draw(canvas);
        return;
    }

    if (hasDialog()) {
        drawHud(canvas);
        return;
    }

    m_chain.draw(canvas, false);
    m_level->drawForeground(canvas);
    m_chain.draw(canvas, true);
    m_bullets.draw(canvas);
    m_texts.draw(canvas);
    drawHud(canvas);
    if (treasureVisible())
        drawTreasure(canvas);
}

void Game::drawPit(Canvas &canvas) const
{
    const QPointF end = m_level->path().end();
    canvas.drawCentered(Assets::ObjectsTexture, QRect(629, 0, 132, 132), end);
    m_pit.draw(canvas, end);
}

void Game::drawHud(Canvas &canvas) const
{
    canvas.drawCentered(Assets::HudTexture, QRect(0, 51, 1280, 730), QPointF(640, 359));

    canvas.text(Assets::Cancun12, QStringLiteral("%1").arg(m_score, 9), QPointF(545, 6), Yellow);
    canvas.text(Assets::Cancun12, QStringLiteral("lvl1-%1").arg(m_levelIndex + 1), QPointF(460, 6),
                Yellow, TextCenter);

    if (m_gauge.width() > 0)
        canvas.drawCentered(Assets::HudTexture, m_gauge,
                            QPointF(820 - (GaugeWidth - m_gauge.width()) / 2, 16));

    m_menuButton.draw(canvas);

    if (m_stats && m_outroEnded)
        m_stats->draw(canvas);
    if (m_menu)
        m_menu->draw(canvas);
    if (m_gameOver)
        m_gameOver->draw(canvas);
}

void Game::drawIntro(Canvas &canvas) const
{
    m_sweep.particles.draw(canvas);

    canvas.text(Assets::NativeAlienExt16, levelName(), QPointF(1030, 625), Color(), TextCenter,
                qMax(0.0f, m_subheaderScale));
    canvas.text(Assets::NativeAlien48, QStringLiteral("LEVEL %1-%2").arg(m_levelIndex + 1).arg(m_difficulty + 1),
                QPointF(1030, 520), Color(), TextCenter, qMax(0.0f, m_headerScale));
}

void Game::drawTreasure(Canvas &canvas) const
{
    const QPointF center = treasurePosition();
    Sprite shadow = m_treasure.sprite;
    shadow.color = Color(0, 0, 0, m_treasure.sprite.color.a / 2);
    shadow.draw(canvas, center + QPointF(-12, 16));
    m_treasure.sprite.draw(canvas, center);
}
