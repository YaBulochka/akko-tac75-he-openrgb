#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import struct
import sys
from pathlib import Path


HERE = Path(__file__).resolve().parent
HOOK_BIN = HERE / "hook.bin"

EXPECTED_HOOK_SHA256 = (
    "d7f71af9a87c09a7d634c4212584030a"
    "16f588e7a59203f34706b791ec518a98"
)

SYMBOLS = {
    "_hook_vendor_dispatch_stub": 0x08025800,
    "handle_vendor_cmd": 0x08025820,
    "boot_init_trampoline": 0x0802596E,
    "frame_copy_gate": 0x08025980,
    "tac75_matrix_to_strip": 0x080259C0,
}

DEFAULT_OUTPUT = (
    HERE / "build" /
    "TAC75_HE_2782_v502_T75D_SAFE_GATE.bin"
)

spec = importlib.util.spec_from_file_location(
    "tac75_hooks",
    HERE / "hooks.py",
)

if spec is None or spec.loader is None:
    raise SystemExit("[FAIL] cannot load hooks.py")

hooks = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = hooks
spec.loader.exec_module(hooks)

parser = argparse.ArgumentParser(
    description="Build the TAC75 HE T75D SAFE-GATE firmware image",
)
parser.add_argument(
    "--stock",
    type=Path,
    default=hooks.FIRMWARE,
    help="exact official TAC75 HE v5.02 stock firmware",
)
parser.add_argument(
    "--output",
    type=Path,
    default=DEFAULT_OUTPUT,
    help="path for the locally patched firmware image",
)
args = parser.parse_args()

hooks.FIRMWARE = args.stock.expanduser().resolve()
OUTPUT = args.output.expanduser().resolve()

from hook_framework import encode_thumb2_bl, encode_thumb2_bw


BOOT_INIT = 0x080152C0
HEAP_BASE_LITERAL = 0x08005528
OLD_HEAP_BASE = 0x20009D98
NEW_HEAP_BASE = 0x20009DA0
PERSISTENT_START = 0x08028000

EXPECTED_BOOT_INIT = bytes.fromhex("4f f4 a4 41")

EXPECTED_MATRIX_TO_STRIP = bytes([
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0xFF,

    0x1D, 0x1C, 0x1B, 0x1A, 0x19, 0x18, 0x17, 0x16,
    0x15, 0x14, 0x13, 0x12, 0x11, 0x10, 0x0F, 0xFF,

    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0xFF,

    0x3A, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33,
    0x32, 0x31, 0x30, 0x2F, 0xFF, 0x2E, 0x2D, 0xFF,

    0x3B, 0xFF, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41,
    0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0xFF,

    0x52, 0xFF, 0x51, 0x50, 0xFF, 0xFF, 0x4F, 0xFF,
    0xFF, 0x4E, 0x4D, 0x4C, 0x4B, 0x4A, 0x49, 0xFF,
])


def fail(message: str) -> None:
    raise SystemExit(f"[FAIL] {message}")


if not HOOK_BIN.is_file():
    fail("hook.bin does not exist")

hook_bin = HOOK_BIN.read_bytes()
hook_digest = hashlib.sha256(hook_bin).hexdigest()

if hook_digest != EXPECTED_HOOK_SHA256:
    fail(f"unexpected hook.bin SHA256: {hook_digest}")

symbols = SYMBOLS.copy()

required_symbols = {
    "_hook_vendor_dispatch_stub",
    "handle_vendor_cmd",
    "boot_init_trampoline",
    "frame_copy_gate",
    "tac75_matrix_to_strip",
}

missing = sorted(required_symbols - symbols.keys())

if missing:
    fail(f"missing ELF symbols: {', '.join(missing)}")

for name in sorted(required_symbols):
    address = symbols[name]

    if not hooks.PATCH_ZONE_START <= address <= hooks.PATCH_ZONE_END:
        fail(
            f"symbol {name} at 0x{address:08X} "
            f"is outside the patch zone"
        )

vendor_stub = symbols["_hook_vendor_dispatch_stub"]
boot_trampoline = symbols["boot_init_trampoline"]
frame_copy_gate = symbols["frame_copy_gate"]

if vendor_stub != hooks.PATCH_ZONE_START:
    fail(
        f"vendor stub is 0x{vendor_stub:08X}, "
        f"expected 0x{hooks.PATCH_ZONE_START:08X}"
    )

patch_capacity = hooks.PATCH_ZONE_END - hooks.PATCH_ZONE_START + 1

if not hook_bin:
    fail("hook.bin is empty")

if len(hook_bin) > patch_capacity:
    fail(
        f"hook.bin is {len(hook_bin)} bytes, "
        f"patch zone capacity is {patch_capacity}"
    )

map_address = symbols["tac75_matrix_to_strip"]
map_offset = map_address - hooks.PATCH_ZONE_START
map_end = map_offset + len(EXPECTED_MATRIX_TO_STRIP)

if map_offset < 0 or map_end > len(hook_bin):
    fail(
        f"LED map range 0x{map_address:08X}-"
        f"0x{map_address + len(EXPECTED_MATRIX_TO_STRIP) - 1:08X} "
        f"is outside hook.bin"
    )

actual_map = hook_bin[map_offset:map_end]

if actual_map != EXPECTED_MATRIX_TO_STRIP:
    fail("embedded 6x16 matrix-to-strip table differs from expected bytes")

physical_leds = [
    value for value in actual_map
    if value != 0xFF
]

if (
    len(physical_leds) != 83
    or len(set(physical_leds)) != 83
    or set(physical_leds) != set(range(83))
):
    fail("embedded LED map is not a complete unique mapping of LEDs 0..82")

stock = hooks.verify_stock()
hooks.verify_exact_bytes(stock)

expected_originals = {
    hooks.VENDOR_DISPATCH: hooks.EXPECTED_VENDOR_PROLOGUE,
    BOOT_INIT: EXPECTED_BOOT_INIT,
    hooks.FRAME_COPY_CALL: hooks.EXPECTED_FRAME_COPY_CALL,
    HEAP_BASE_LITERAL: struct.pack("<I", OLD_HEAP_BASE),
}

for address, expected in expected_originals.items():
    offset = address - hooks.FILE_BASE
    actual = stock[offset:offset + len(expected)]

    if actual != expected:
        fail(
            f"stock bytes at 0x{address:08X} are "
            f"{actual.hex(' ')}, expected {expected.hex(' ')}"
        )

patches = [
    (
        hooks.VENDOR_DISPATCH,
        encode_thumb2_bw(hooks.VENDOR_DISPATCH, vendor_stub),
        "vendor dispatcher B.W",
    ),
    (
        BOOT_INIT,
        encode_thumb2_bw(BOOT_INIT, boot_trampoline),
        "boot initializer B.W",
    ),
    (
        hooks.FRAME_COPY_CALL,
        encode_thumb2_bl(hooks.FRAME_COPY_CALL, frame_copy_gate),
        "frame-copy BL",
    ),
    (
        HEAP_BASE_LITERAL,
        struct.pack("<I", NEW_HEAP_BASE),
        "heap base reservation",
    ),
]

patched = bytearray(stock)
patch_zone_offset = hooks.PATCH_ZONE_START - hooks.FILE_BASE

if len(patched) < patch_zone_offset:
    patched.extend(b"\xFF" * (patch_zone_offset - len(patched)))

patched.extend(
    b"\xFF" * max(
        0,
        patch_zone_offset + len(hook_bin) - len(patched),
    )
)

patched[
    patch_zone_offset:patch_zone_offset + len(hook_bin)
] = hook_bin

allowed_ranges: list[range] = []

for address, replacement, _description in patches:
    offset = address - hooks.FILE_BASE
    patched[offset:offset + len(replacement)] = replacement
    allowed_ranges.append(range(offset, offset + len(replacement)))

for offset, (old, new) in enumerate(zip(stock, patched)):
    if old == new:
        continue

    if not any(offset in allowed for allowed in allowed_ranges):
        fail(
            f"unexpected stock modification at file offset 0x{offset:X}: "
            f"{old:02X} -> {new:02X}"
        )

padding = patched[len(stock):patch_zone_offset]

if any(byte != 0xFF for byte in padding):
    fail("gap between stock firmware and patch zone is not all 0xFF")

actual_hook = patched[
    patch_zone_offset:patch_zone_offset + len(hook_bin)
]

if actual_hook != hook_bin:
    fail("hook.bin was not copied exactly into the patch zone")

expected_size = patch_zone_offset + len(hook_bin)

if len(patched) != expected_size:
    fail(
        f"output size is 0x{len(patched):X}, "
        f"expected 0x{expected_size:X}"
    )

output_end = hooks.FILE_BASE + len(patched)

if output_end > PERSISTENT_START:
    fail(
        f"output reaches 0x{output_end:08X}, "
        f"persistent storage begins at 0x{PERSISTENT_START:08X}"
    )

OUTPUT.parent.mkdir(parents=True, exist_ok=True)
OUTPUT.write_bytes(patched)

digest = hashlib.sha256(patched).hexdigest()
app_size = len(patched) - 0x5000

print()
print("[PASS] exact official TAC75 HE v5.02 stock image")
print("[PASS] hook.bin copied unchanged into patch zone")
print("[PASS] embedded LED map: exact 6x16 table, LEDs 0..82 unique")
print("[PASS] all required ELF symbols are inside the patch zone")
print("[PASS] unused gap padded entirely with 0xFF")
print("[PASS] no unexpected modifications inside stock firmware")

for address, replacement, description in patches:
    original = expected_originals[address]
    print(
        f"[PASS] 0x{address:08X} {description}: "
        f"{original.hex(' ')} -> {replacement.hex(' ')}"
    )

print(
    f"[PASS] reserved SRAM: "
    f"0x{OLD_HEAP_BASE:08X}-0x{NEW_HEAP_BASE - 1:08X}"
)
print(
    f"[PASS] patched image ends at 0x{output_end - 1:08X}, "
    f"below persistent storage"
)
print()
print(f"Vendor stub:       0x{vendor_stub:08X}")
print(f"Boot trampoline:   0x{boot_trampoline:08X}")
print(f"Frame-copy gate:   0x{frame_copy_gate:08X}")
print(f"LED map:           0x{symbols['tac75_matrix_to_strip']:08X}")
print(f"Hook size:         {len(hook_bin)} bytes (0x{len(hook_bin):X})")
print(f"Full image size:   {len(patched)} bytes (0x{len(patched):X})")
print(f"App transfer size: {app_size} bytes (0x{app_size:X})")
print(f"SHA256:            {digest}")
print(f"Output:            {OUTPUT}")
print()
print("T75D DIRECT MODE SAFE-GATE VERIFIED")
print("NOTHING WAS FLASHED")
