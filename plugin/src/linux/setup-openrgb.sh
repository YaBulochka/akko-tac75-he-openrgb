#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "$0")" && pwd)"
cd "$ROOT"

if [[ ! -d OpenRGB/.git ]]; then
    echo "==> Cloning fresh OpenRGB master..."
    git clone --depth 1 https://github.com/CalcProgrammer1/OpenRGB.git OpenRGB
else
    echo "==> Updating OpenRGB..."
    git -C OpenRGB fetch --depth 1 origin master
    git -C OpenRGB reset --hard origin/master
fi

echo "==> OpenRGB commit: $(git -C OpenRGB rev-parse --short HEAD)"
echo "==> Plugin API: $(grep -E '^#define OPENRGB_PLUGIN_API_VERSION' OpenRGB/OpenRGBPluginInterface.h || true)"
