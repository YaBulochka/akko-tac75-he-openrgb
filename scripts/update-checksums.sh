#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$ROOT"

sha256sum \
    ./build-firmware.sh \
    ./.gitignore \
    ./LICENSE \
    ./licenses/iot_driver-GPL-3.0.txt \
    ./licenses/openrgb-plugin-GPL-2.0.txt \
    ./patcher/build_only.py \
    ./patcher/hook.bin \
    ./patcher/hook_framework.py \
    ./patcher/hooks.py \
    ./plugin/bin/linux/libOpenRGBTAC75HEPlugin.so \
    ./README.md \
    ./sources/iot_driver-0.1.0-modified-source.tar.zst \
    ./sources/OpenRGB-TAC75HE-Plugin-modified-source.tar.zst \
    ./sources/TAC75HE-T75D-hook-source.tar.zst \
    ./tools/iot_driver \
    ./udev/70-akko-tac75-he.rules \
    > SHA256SUMS

echo "==> SHA256SUMS updated"
