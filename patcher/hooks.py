#!/usr/bin/env python3

from __future__ import annotations

import hashlib
import sys
from pathlib import Path


HERE = Path(__file__).resolve().parent
FIRMWARE = HERE.parent / "firmware_stock.bin"
GENERATED = HERE / "hooks_gen.S"

sys.path.insert(0, str(HERE))

from hook_framework import Hook, HookEngine


EXPECTED_SIZE = 0x20A40
EXPECTED_SHA256 = (
    "2e188151b6025aac685855c6088959fa3"
    "d9c9058d9055bfb07a92c380b68f306"
)

FILE_BASE = 0x08000000

PATCH_ZONE_START = 0x08025800
PATCH_ZONE_END = 0x08027FFF

VENDOR_DISPATCH = 0x08012850
FRAME_COPY_CALL = 0x0801539E

EXPECTED_VENDOR_PROLOGUE = bytes.fromhex("2d e9 f0 47")
EXPECTED_FRAME_COPY_CALL = bytes.fromhex("f0 f7 61 f9")


def fail(message: str) -> None:
    raise SystemExit(f"[FAIL] {message}")


def verify_stock() -> bytes:
    data = FIRMWARE.read_bytes()

    if len(data) != EXPECTED_SIZE:
        fail(
            f"stock size is 0x{len(data):X}, "
            f"expected 0x{EXPECTED_SIZE:X}"
        )

    digest = hashlib.sha256(data).hexdigest()

    if digest != EXPECTED_SHA256:
        fail(f"unexpected stock SHA256: {digest}")

    if data[0x5000:0x5010] != b"AT32F405 8KMKB  ":
        fail("application header mismatch at file offset 0x5000")

    return data


def verify_exact_bytes(data: bytes) -> None:
    vendor_off = VENDOR_DISPATCH - FILE_BASE
    vendor_bytes = data[vendor_off:vendor_off + 4]

    if vendor_bytes != EXPECTED_VENDOR_PROLOGUE:
        fail(
            f"vendor dispatcher bytes are {vendor_bytes.hex(' ')}, "
            f"expected {EXPECTED_VENDOR_PROLOGUE.hex(' ')}"
        )

    frame_off = FRAME_COPY_CALL - FILE_BASE
    frame_bytes = data[frame_off:frame_off + 4]

    if frame_bytes != EXPECTED_FRAME_COPY_CALL:
        fail(
            f"frame-copy call bytes are {frame_bytes.hex(' ')}, "
            f"expected {EXPECTED_FRAME_COPY_CALL.hex(' ')}"
        )

    print(
        f"[PASS] vendor dispatcher 0x{VENDOR_DISPATCH:08X}: "
        f"{vendor_bytes.hex(' ')}"
    )
    print(
        f"[PASS] frame-copy call  0x{FRAME_COPY_CALL:08X}: "
        f"{frame_bytes.hex(' ')}"
    )


def create_engine() -> HookEngine:
    engine = HookEngine(
        FIRMWARE,
        file_base=FILE_BASE,
        patch_zone_start=PATCH_ZONE_START,
        patch_zone_end=PATCH_ZONE_END,
    )

    engine.add_hook(
        Hook(
            name="vendor_dispatch",
            target=VENDOR_DISPATCH,
            handler="handle_vendor_cmd",
            mode="filter",
            displace=4,
        )
    )

    return engine


def validate() -> HookEngine:
    data = verify_stock()
    print("[PASS] exact TAC75 HE 2782 v5.02 stock image")

    verify_exact_bytes(data)

    engine = create_engine()

    print(engine.summary())
    print()
    print("RESULT: TAC75 PATCH DEFINITION VALIDATED")
    print("Nothing was patched or flashed.")

    return engine


def generate() -> None:
    engine = validate()
    engine.generate(GENERATED)

    print()
    print(f"Generated assembly: {GENERATED}")
    print("No firmware image was created.")


def main() -> None:
    if len(sys.argv) != 2 or sys.argv[1] not in {"validate", "generate"}:
        raise SystemExit(
            f"Usage: {Path(sys.argv[0]).name} validate|generate"
        )

    if sys.argv[1] == "validate":
        validate()
    else:
        generate()


if __name__ == "__main__":
    main()
