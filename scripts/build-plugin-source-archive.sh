#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "$0")/.." && pwd)"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

VERSION="$(python3 -c \
    'import json,sys; print(json.load(open(sys.argv[1]))["Version"])' \
    "$ROOT/plugin/src/common/OpenRGBTAC75HEPlugin.json")"

DEST="$STAGE/OpenRGB-TAC75HE-Plugin-$VERSION"

mkdir -p \
    "$DEST/plugin/src/common" \
    "$DEST/plugin/src/linux/OpenRGB/RGBController" \
    "$DEST/plugin/src/linux/OpenRGB/dependencies/json/nlohmann" \
    "$DEST/licenses"

cp "$ROOT/plugin/src/common/"* "$DEST/plugin/src/common/"

cp \
    "$ROOT/plugin/src/linux/OpenRGBTAC75HEPlugin.pro" \
    "$ROOT/plugin/src/linux/build.sh" \
    "$ROOT/plugin/src/linux/setup-openrgb.sh" \
    "$DEST/plugin/src/linux/"

cp \
    "$ROOT/plugin/src/linux/OpenRGB/OpenRGBPluginInterface.h" \
    "$ROOT/plugin/src/linux/OpenRGB/filesystem.h" \
    "$DEST/plugin/src/linux/OpenRGB/"

cp \
    "$ROOT/plugin/src/linux/OpenRGB/RGBController/RGBControllerInterface.h" \
    "$DEST/plugin/src/linux/OpenRGB/RGBController/"

cp \
    "$ROOT/plugin/src/linux/OpenRGB/dependencies/json/nlohmann/json.hpp" \
    "$DEST/plugin/src/linux/OpenRGB/dependencies/json/nlohmann/"

cp "$ROOT/README.md" "$ROOT/LICENSE" "$DEST/"
cp "$ROOT/licenses/openrgb-plugin-GPL-2.0.txt" "$DEST/licenses/"

OUT="$ROOT/sources/OpenRGB-TAC75HE-Plugin-modified-source.tar.zst"

tar --zstd --sort=name --mtime='UTC 2026-10-20' \
    --owner=0 --group=0 --numeric-owner \
    -cf "$OUT" -C "$STAGE" "OpenRGB-TAC75HE-Plugin-$VERSION"

echo "==> Wrote $OUT"
echo "==> SHA256: $(sha256sum "$OUT" | cut -d' ' -f1)"
