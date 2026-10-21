# SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
# SPDX-License-Identifier: MIT

TARGET = ru.kln0x6b.AurZuma

CONFIG += \
    auroraapp \
    auroraapp_i18n \
    c++14

QT += multimedia

PKGCONFIG += \
    vorbisfile \

INCLUDEPATH += src

HEADERS += \
    src/engine/audio.h \
    src/engine/bitmapfont.h \
    src/engine/canvas.h \
    src/engine/canvasnode.h \
    src/engine/engine.h \
    src/engine/oggstream.h \
    src/engine/pointer.h \
    src/engine/storage.h \
    src/engine/texturestore.h \
    src/game/assets.h \
    src/game/ballchain.h \
    src/game/bullets.h \
    src/game/effects.h \
    src/game/frog.h \
    src/game/game.h \
    src/game/levels.h \
    src/game/sprite.h \
    src/ui/dialog.h \
    src/ui/menuscreen.h \
    src/ui/widgets.h \
    src/gameview.h \
    src/zuma.h \

SOURCES += \
    src/engine/audio.cpp \
    src/engine/bitmapfont.cpp \
    src/engine/canvas.cpp \
    src/engine/canvasnode.cpp \
    src/engine/engine.cpp \
    src/engine/oggstream.cpp \
    src/engine/storage.cpp \
    src/engine/texturestore.cpp \
    src/game/assets.cpp \
    src/game/ballchain.cpp \
    src/game/bullets.cpp \
    src/game/effects.cpp \
    src/game/frog.cpp \
    src/game/game.cpp \
    src/game/levels.cpp \
    src/game/sprite.cpp \
    src/ui/dialog.cpp \
    src/ui/menuscreen.cpp \
    src/ui/widgets.cpp \
    src/gameview.cpp \
    src/main.cpp \
    src/zuma.cpp \

data.files = data
data.path = /usr/share/$${TARGET}

manifest.files = rpm/manifest.json
manifest.path = /

INSTALLS += data manifest

DISTFILES += \
    rpm/$${TARGET}.spec \
    rpm/manifest.json \
    tools/import_assets.sh \
    tools/release.sh \
    README.md \
    LICENSE \

AURORAAPP_ICONS = 86x86 108x108 128x128 172x172

TRANSLATIONS += \
    translations/$${TARGET}.ts \
    translations/$${TARGET}-ru.ts \
