#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "$0")" && pwd)"
cd "$ROOT"

mkdir -p "$ROOT/../../bin/linux"
OUT="$(cd -- "$ROOT/../../bin/linux" && pwd)"

command -v pkg-config >/dev/null || { echo 'pkg-config is required'; exit 1; }

pkg-config --exists hidapi-hidraw || {
    echo 'hidapi-hidraw pkg-config module not found (Arch: install hidapi)'
    exit 1
}

# Prefer the Qt major version used by the installed OpenRGB binary.  A plugin
# built against Qt 5 will not load into a Qt 6 OpenRGB, and vice versa.
openrgb_qt_major()
{
    local openrgb_bin

    command -v ldd >/dev/null 2>&1 || return 0
    openrgb_bin="$(command -v openrgb 2>/dev/null || true)"
    [[ -n "$openrgb_bin" ]] || return 0

    if ldd "$openrgb_bin" 2>/dev/null | grep -q 'libQt6Core'; then
        echo 6
    elif ldd "$openrgb_bin" 2>/dev/null | grep -q 'libQt5Core'; then
        echo 5
    fi
}

QMAKE=""
ORGB_QT_MAJOR="$(openrgb_qt_major)"

if [[ -n "$ORGB_QT_MAJOR" ]]; then
    for q in "qmake$ORGB_QT_MAJOR" "qmake-qt$ORGB_QT_MAJOR"; do
        if command -v "$q" >/dev/null 2>&1; then
            QMAKE="$q"
            break
        fi
    done
fi

if [[ -z "$QMAKE" ]]; then
    for q in qmake6 qmake-qt6 qmake; do
        if command -v "$q" >/dev/null 2>&1; then
            QMAKE="$q"
            break
        fi
    done
fi

[[ -n "$QMAKE" ]] || { echo 'qmake/qmake6 not found'; exit 1; }

QMAKE_VERSION="$("$QMAKE" -query QT_VERSION)"
echo "==> qmake: $QMAKE (Qt $QMAKE_VERSION)"

if [[ -n "$ORGB_QT_MAJOR" && "${QMAKE_VERSION%%.*}" != "$ORGB_QT_MAJOR" ]]; then
    echo "warning: installed OpenRGB uses Qt $ORGB_QT_MAJOR, but $QMAKE uses Qt $QMAKE_VERSION"
    echo "warning: install the matching Qt development package or the plugin will not load"
fi

rm -rf build
mkdir build
cd build
"$QMAKE" ../OpenRGBTAC75HEPlugin.pro
make -j"$(nproc)"

SO="$OUT/libOpenRGBTAC75HEPlugin.so"

[[ -f "$SO" ]] || {
    echo "error: $SO was not produced"
    exit 1
}

echo
echo "==> BUILT: $SO"
