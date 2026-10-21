#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Kirill Koleno <kln0x6b@support.ru>
# SPDX-License-Identifier: MIT
#
# Builds signed packages for every supported architecture, tags the release
# and pushes the tag to GitHub. Prints the packages and the release notes
# (the latest entry of the RPM changelog) to attach to the GitHub release.
# Requires Aurora SDK (Build Tools).
#
# Usage: tools/release.sh [--build-only]

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SDK="${AURORA_SDK:-$HOME/AuroraOS/sdk/5.2.1.200}"
PACKAGE="ru.kln0x6b.AurZuma"
VERSION="$(awk '/^Version:/ { print $2 }' "$ROOT/rpm/$PACKAGE.spec")"
RELEASE="$(awk '/^Release:/ { print $2 }' "$ROOT/rpm/$PACKAGE.spec")"
TAG="v$VERSION"
[ "$RELEASE" = 1 ] || TAG="$TAG-$RELEASE"

# The SDK builds inside a container that sees the parent directory as /workspace
export WORKSPACE_DIR="$(dirname "$ROOT")"
PROJECT="$(basename "$ROOT")"

packages=()
for arch in armv7hl aarch64; do
    case "$arch" in
        armv7hl) flag=--arm32 ;;
        aarch64) flag=--arm64 ;;
    esac
    dir="$ROOT/build/$arch"
    rm -rf "$dir/RPMS"
    mkdir -p "$dir"
    (cd "$dir" && "$SDK/tools/apptool" build "$flag" \
        --srcdir "/workspace/$PROJECT" --dstdir "/workspace/$PROJECT/build/$arch")
    packages+=("$dir/RPMS/$PACKAGE-$VERSION-$RELEASE.$arch.rpm")
done

if [ "${1:-}" = --build-only ]; then
    printf '%s\n' "${packages[@]}"
    exit 0
fi

if ! git -C "$ROOT" rev-parse -q --verify "refs/tags/$TAG" >/dev/null; then
    git -C "$ROOT" tag -a "$TAG" -m "Zuma Deluxe AuroraOS $VERSION-$RELEASE"
fi
git -C "$ROOT" push origin "$TAG"

echo
echo "Tag $TAG is pushed. Create the release on GitHub:"
echo "  https://github.com/kln0x6B/Zuma-Deluxe-AuroraOS/releases/new?tag=$TAG"
echo
echo "Title:"
echo "  Zuma Deluxe AuroraOS $VERSION"
echo
echo "Description:"
awk 'NR == 1 { next } /^\*/ { exit } NF { print "  " $0 }' "$ROOT/rpm/$PACKAGE.changes"
echo
echo "Attach the packages:"
printf '  %s\n' "${packages[@]}"
