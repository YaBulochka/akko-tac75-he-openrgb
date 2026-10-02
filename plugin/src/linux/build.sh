#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd -- "$(dirname -- "$0")" && pwd)"
cd "$ROOT"

command -v pkg-config >/dev/null || { echo 'pkg-config is required'; exit 1; }
pkg-config --exists hidapi-hidraw || {
    echo 'hidapi-hidraw pkg-config module not found (Arch: install hidapi)'
    exit 1
}

QMAKE=""
for q in qmake6 qmake; do
    if command -v "$q" >/dev/null 2>&1; then
        QMAKE="$q"
        break
    fi
done
[[ -n "$QMAKE" ]] || { echo 'qmake/qmake6 not found'; exit 1; }

rm -rf build
mkdir build
cd build
"$QMAKE" ../OpenRGBTAC75HEPlugin.pro
make -j"$(nproc)"

echo
echo "==> BUILT: $PWD/libOpenRGBTAC75HEPlugin.so"
