#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
OUTPUT="${1:-${PWD}/TAC75_HE_2782_v502_T75D_SAFE_GATE.bin}"

EXPECTED_STOCK_SHA='2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306'
EXPECTED_PATCHED_SHA='757c154ce58e656f68aca476daf68e76d1a7c4a40cc1d452b0b484ec4718e256'

TMPDIR_BUILD="$(mktemp -d)"
trap 'rm -rf -- "$TMPDIR_BUILD"' 0 HUP INT TERM

STOCK="${TMPDIR_BUILD}/TAC75_HE_2782_v502_stock.bin"

printf '%s\n' '=== Downloading official TAC75 HE v5.02 firmware ==='

"${ROOT}/tools/iot_driver" \
    firmware download \
    --device-id 2782 \
    --output "$STOCK"

STOCK_SHA="$(sha256sum "$STOCK" | awk '{print $1}')"

if [ "$STOCK_SHA" != "$EXPECTED_STOCK_SHA" ]; then
    printf '[FAIL] unexpected stock SHA256: %s\n' "$STOCK_SHA" >&2
    false
fi

printf '%s\n\n' '[PASS] official stock firmware verified'
printf '%s\n' '=== Building T75D SAFE-GATE firmware locally ==='

python3 -B "${ROOT}/patcher/build_only.py" \
    --stock "$STOCK" \
    --output "$OUTPUT"

PATCHED_SHA="$(sha256sum "$OUTPUT" | awk '{print $1}')"

if [ "$PATCHED_SHA" != "$EXPECTED_PATCHED_SHA" ]; then
    printf '[FAIL] unexpected patched SHA256: %s\n' "$PATCHED_SHA" >&2
    false
fi

printf '\n%s\n' '[PASS] T75D SAFE-GATE firmware built successfully'
printf 'Output: %s\n' "$OUTPUT"
printf 'SHA256: %s\n\n' "$PATCHED_SHA"
printf '%s\n' 'Nothing was flashed.'
