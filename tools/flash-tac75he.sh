#!/usr/bin/env bash
#
# Safety wrapper for flashing the Akko TAC75 HE.
#
# It verifies the firmware SHA256, the USB VID:PID, the device ID and the
# reported firmware version before calling tools/iot_driver.

set -euo pipefail

ROOT="$(cd -- "$(dirname -- "$0")/.." && pwd)"
DRIVER="${ROOT}/tools/iot_driver"

PATCHED_SHA='757c154ce58e656f68aca476daf68e76d1a7c4a40cc1d452b0b484ec4718e256'
STOCK_SHA='2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306'

mode='patched'
file=''

usage()
{
    cat <<EOF
Usage: $(basename "$0") [--stock] <firmware.bin>

Flashes a locally built TAC75 HE image after checking:

  - the file SHA256 matches the known patched or stock image;
  - a TAC75 HE (3151:502D) is connected in normal mode;
  - the keyboard reports device ID 2782 and firmware v5.02.

Use --stock when flashing the unmodified stock image for restore.

Close OpenRGB and the Akko Web Driver first. Do not disconnect the keyboard
while flashing is in progress.

If the keyboard is already in bootloader mode (3151:502A), reconnect it in
normal mode before using this wrapper.
EOF
}

fail()
{
    printf '[FAIL] %s\n' "$*" >&2
    exit 1
}

while (($#)); do
    case "$1" in
        --stock)
            mode='stock'
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            break
            ;;
        -*)
            printf 'Unknown option: %s\n' "$1" >&2
            usage >&2
            exit 2
            ;;
        *)
            if [[ -n "$file" ]]; then
                fail "only one firmware file may be specified"
            fi
            file="$1"
            ;;
    esac
    shift
done

if [[ -z "$file" && $# -gt 0 ]]; then
    file="$1"
fi

if [[ -z "$file" ]]; then
    usage >&2
    exit 2
fi

for tool in sha256sum awk mktemp tee lsusb grep; do
    command -v "$tool" >/dev/null 2>&1 || fail "required tool not found: $tool"
done

if [[ ! -f "$file" ]]; then
    fail "firmware file not found: $file"
fi

if [[ ! -x "$DRIVER" ]]; then
    fail "flashing utility is missing or not executable: $DRIVER"
fi

if [[ "$mode" == 'stock' ]]; then
    expected_sha="$STOCK_SHA"
else
    expected_sha="$PATCHED_SHA"
fi

actual_sha="$(sha256sum "$file" | awk '{print $1}')"

if [[ "$actual_sha" != "$expected_sha" ]]; then
    printf '[FAIL] SHA256 mismatch for %s\n' "$file" >&2
    printf '       got:      %s\n' "$actual_sha" >&2
    printf '       expected: %s (%s image)\n' "$expected_sha" "$mode" >&2
    exit 1
fi

printf '[PASS] %s image SHA256 verified\n' "$mode"

if ! lsusb -d 3151:502d >/dev/null 2>&1; then
    if lsusb -d 3151:502a >/dev/null 2>&1; then
        fail "keyboard is in bootloader mode; reconnect it in normal mode"
    fi

    fail "TAC75 HE (3151:502D) not found; connect it in normal mode"
fi

printf '[PASS] TAC75 HE found at 3151:502D\n'

if pgrep -x openrgb >/dev/null 2>&1; then
    printf '[WARN] OpenRGB is running. Close it before flashing.\n' >&2
    read -r -p 'Continue anyway? [y/N] ' reply
    [[ "$reply" =~ ^[Yy]$ ]] || exit 1
fi

printf 'Reading device ID and firmware version...\n'

info_output="$("$DRIVER" info 2>&1)" || {
    printf '%s\n' "$info_output" >&2
    fail "could not read device information; close OpenRGB and try again"
}

printf '%s\n' "$info_output"

grep -Eq '^Device ID:[[:space:]]+2782([[:space:]]|$)' <<<"$info_output" \
    || fail "unexpected device ID; expected 2782"

grep -Eq '^Firmware:[[:space:]]+v5\.02([[:space:]]|$)' <<<"$info_output" \
    || fail "unexpected firmware version; expected v5.02"

printf '[PASS] device ID 2782, firmware v5.02\n'
printf '\n'
printf 'Firmware: %s\n' "$file"
printf 'SHA256:   %s\n' "$actual_sha"
printf 'This will overwrite the keyboard firmware.\n'

read -r -p "Type 'yes' to continue: " reply
if [[ "$reply" != 'yes' ]]; then
    printf 'Aborted.\n'
    exit 1
fi

log="$(mktemp)"
trap 'rm -f -- "$log"' 0

set +e
"$DRIVER" firmware flash --device-id 2782 --yes "$file" 2>&1 | tee "$log"
rc=${PIPESTATUS[0]}
set -e

if (( rc != 0 )); then
    fail "flashing utility exited with status $rc"
fi

if ! grep -q 'Flash complete!' "$log"; then
    fail "flashing did not report success; do not disconnect the keyboard"
fi

if grep -q 'Flash failed' "$log"; then
    fail "flashing reported an error; do not disconnect the keyboard"
fi

printf '\n'
printf '[PASS] flash complete\n'
printf 'Reconnect the keyboard and verify the device before normal use.\n'
