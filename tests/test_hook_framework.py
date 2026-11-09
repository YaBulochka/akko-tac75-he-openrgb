import re
import sys
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "patcher"))

import hook_framework as hf


def extract_table(source: str, declaration: str) -> list[int]:
    start = source.find(declaration)
    if start < 0:
        raise AssertionError(f"table not found: {declaration}")

    brace = source.find("{", start)
    if brace < 0:
        raise AssertionError(f"table body not found: {declaration}")

    depth = 0
    end = -1

    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                end = index
                break

    if end < 0:
        raise AssertionError(f"unterminated table: {declaration}")

    tokens = re.findall(r"0x([0-9A-Fa-f]{2})|\bNA\b", source[brace + 1 : end])
    values = []
    for hex_value in tokens:
        values.append(0xFF if hex_value == "" else int(hex_value, 16))
    return values


class ThumbEncodingTests(unittest.TestCase):
    def test_encode_bw_vendor_dispatch(self) -> None:
        self.assertEqual(
            hf.encode_thumb2_bw(0x08012850, 0x08025800),
            bytes.fromhex("12 f0 d6 bf"),
        )

    def test_encode_bw_boot_init(self) -> None:
        self.assertEqual(
            hf.encode_thumb2_bw(0x080152C0, 0x0802596E),
            bytes.fromhex("10 f0 55 bb"),
        )

    def test_encode_bl_frame_copy(self) -> None:
        self.assertEqual(
            hf.encode_thumb2_bl(0x0801539E, 0x08025980),
            bytes.fromhex("10 f0 ef fa"),
        )


class InstructionDecodeTests(unittest.TestCase):
    def test_decodes_32bit_push(self) -> None:
        instructions = hf.decode_instructions(
            bytes.fromhex("2d e9 f0 47"),
            0x08012850,
        )
        self.assertEqual(len(instructions), 1)
        self.assertEqual(instructions[0]["size"], 4)

    def test_decodes_16bit_bx_lr(self) -> None:
        instructions = hf.decode_instructions(
            bytes.fromhex("70 47"),
            0x08000000,
        )
        self.assertEqual(len(instructions), 1)
        self.assertEqual(instructions[0]["size"], 2)


class LedMapTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.plugin_source = (
            ROOT / "plugin/src/common/RGBController_TAC75HE.cpp"
        ).read_text()
        cls.build_source = (ROOT / "patcher/build_only.py").read_text()

    def test_visual_map_is_complete(self) -> None:
        values = extract_table(self.plugin_source, "tac75_matrix_map[6][16]")
        physical = [value for value in values if value != 0xFF]

        self.assertEqual(len(values), 96)
        self.assertEqual(len(physical), 83)
        self.assertEqual(set(physical), set(range(83)))

    def test_transport_map_is_complete(self) -> None:
        values = extract_table(self.plugin_source, "tac75_transport_map[96]")
        physical = [value for value in values if value != 0xFF]

        self.assertEqual(len(values), 96)
        self.assertEqual(len(physical), 83)
        self.assertEqual(set(physical), set(range(83)))

    def test_plugin_transport_map_matches_firmware_table(self) -> None:
        plugin_values = extract_table(
            self.plugin_source,
            "tac75_transport_map[96]",
        )

        match = re.search(
            r"EXPECTED_MATRIX_TO_STRIP\s*=\s*bytes\(\[(.*?)\]\)",
            self.build_source,
            re.S,
        )
        self.assertIsNotNone(match)

        firmware_values = [
            int(value, 16)
            for value in re.findall(r"0x([0-9A-Fa-f]{2})", match.group(1))
        ]

        self.assertEqual(len(firmware_values), 96)
        self.assertEqual(plugin_values, firmware_values)


if __name__ == "__main__":
    unittest.main()
