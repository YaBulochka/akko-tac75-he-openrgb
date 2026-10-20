#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "$0")" && pwd)"
TAG="${1:-release_1.1}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "==> Fetching OpenRGB $TAG plugin headers..."

git clone --quiet --depth 1 --branch "$TAG" \
    https://github.com/CalcProgrammer1/OpenRGB.git \
    "$TMP/OpenRGB"

install -Dm644 \
    "$TMP/OpenRGB/OpenRGBPluginInterface.h" \
    "$ROOT/OpenRGB/OpenRGBPluginInterface.h"

install -Dm644 \
    "$TMP/OpenRGB/RGBController/RGBControllerInterface.h" \
    "$ROOT/OpenRGB/RGBController/RGBControllerInterface.h"

install -Dm644 \
    "$TMP/OpenRGB/filesystem.h" \
    "$ROOT/OpenRGB/filesystem.h"

install -Dm644 \
    "$TMP/OpenRGB/dependencies/json/nlohmann/json.hpp" \
    "$ROOT/OpenRGB/dependencies/json/nlohmann/json.hpp"

echo "==> Plugin API: $(grep -E '^#define OPENRGB_PLUGIN_API_VERSION' "$ROOT/OpenRGB/OpenRGBPluginInterface.h")"
echo "==> Headers synced from $TAG"
