#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2023 GalaxyShad
# SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
# SPDX-License-Identifier: MIT
# Based on Zuma Deluxe HD: https://github.com/GalaxyShad/Zuma-Deluxe-HD
#
# Imports game assets from a Zuma-Deluxe-HD checkout into data/.
# Requires openmpt123 and oggenc to render the MO3 soundtrack.
#
# Usage: tools/import_assets.sh <path-to-Zuma-Deluxe-HD>

set -euo pipefail

SRC="${1:?path to Zuma-Deluxe-HD checkout is required}/content"
DST="$(cd "$(dirname "$0")/.." && pwd)/data"

for tool in openmpt123 oggenc; do
    command -v "$tool" >/dev/null || { echo "$tool is required" >&2; exit 1; }
done

rm -rf "$DST"
mkdir -p "$DST"/{images/thumbnails,fonts,levels,sounds,music}

cp "$SRC"/images/{frog.png,gameobjects.png,gameinterface.png,menu.png,disclaimer.jpg} "$DST/images/"
for i in $(seq 1 18); do
    cp "$SRC/images/thumbnails/thumb ($i).jpg" "$DST/images/thumbnails/$(printf %02d "$i").jpg"
done

for font in cancun10 cancun12 cancunfloat14 nativealien48 nativealienextended16 nativealienextended18; do
    cp "$SRC/fonts/$font".{png,txt} "$DST/fonts/"
done

cp "$SRC/levels/levels.xml" "$DST/levels/"
for level in spiral claw riverbed targetglyph turnaround longrange tiltspiral underover warshak \
             loopy groovefest spaceinvaders triangle coaster squaresville tunnellevel overunder inversespiral; do
    cp -R "$SRC/levels/$level" "$DST/levels/"
done

for sound in ballclick1 ballclick2 ballsdestroyed1 ballsdestroyed2 ballsdestroyed3 ballsdestroyed4 \
             ballsdestroyed5 button1 button2 chant1 chant2 chant4 chant8 chime1 choral1 \
             coingrab endoflevelpop1 fireball1 gemvanishes jewelappear lighttrail2 rolling; do
    cp "$SRC/sounds/$sound.ogg" "$DST/sounds/"
done

# Each track of the soundtrack is a separate subsong of zuma.mo3
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
render() {
    openmpt123 --batch --quiet --force --no-float --subsong "$1" --samplerate 44100 --channels 2 \
        --output "$WORK/$2.wav" -- "$SRC/music/zuma.mo3"
    oggenc --quiet -q 3 -o "$DST/music/$2.ogg" "$WORK/$2.wav"
}
render 0 game
render 2 menu
render 4 gauntlet
render 6 win
render 7 gameover

find "$DST" -type f -exec chmod 644 {} +
echo "Assets imported into $DST"
