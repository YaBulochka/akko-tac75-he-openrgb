#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
OUTPUT="${1:-${PWD}/TAC75_HE_2782_v502_T75D_SAFE_GATE.bin}"

EXPECTED_STOCK_SHA='2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306'
EXPECTED_PATCHED_SHA='757c154ce58e656f68aca476daf68e76d1a7c4a40cc1d452b0b484ec4718e256'

for tool in python3 sha256sum awk mktemp; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf '[FAIL] required tool not found: %s\n' "$tool" >&2
        exit 1
    fi
done

if [ ! -x "${ROOT}/tools/iot_driver" ]; then
    printf '[FAIL] %s is missing or not executable\n' "${ROOT}/tools/iot_driver" >&2
    printf '       run: chmod +x tools/iot_driver\n' >&2
    exit 1
fi

if [ -d "$OUTPUT" ]; then
    printf '[FAIL] output path is a directory: %s\n' "$OUTPUT" >&2
    exit 1
fi

TMPDIR_BUILD="$(mktemp -d)"
OUTPUT_TMP="${OUTPUT}.tmp.$$"

cleanup()
{
    rm -rf -- "$TMPDIR_BUILD" "$OUTPUT_TMP"
}

trap cleanup 0
trap 'exit 130' HUP INT TERM

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
printf 'Temporary image: %s\n' "$OUTPUT_TMP"

python3 -B "${ROOT}/patcher/build_only.py" \
    --stock "$STOCK" \
    --output "$OUTPUT_TMP"

PATCHED_SHA="$(sha256sum "$OUTPUT_TMP" | awk '{print $1}')"

if [ "$PATCHED_SHA" != "$EXPECTED_PATCHED_SHA" ]; then
    printf '[FAIL] unexpected patched SHA256: %s\n' "$PATCHED_SHA" >&2
    false
fi

mv -f -- "$OUTPUT_TMP" "$OUTPUT"

printf '\n%s\n' '[PASS] T75D SAFE-GATE firmware built successfully'
printf 'Output: %s\n' "$OUTPUT"
printf 'SHA256: %s\n\n' "$PATCHED_SHA"
printf '%s\n' 'Nothing was flashed.'
