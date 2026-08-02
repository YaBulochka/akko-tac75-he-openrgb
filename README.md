# Akko TAC75 HE OpenRGB Support

Unofficial and experimental OpenRGB Direct Mode support for the **Akko TAC75 HE**.

This release contains:

- a POSIX shell script that downloads the official stock firmware and applies the T75D patch locally;
- an OpenRGB plugin;
- a reverse-engineered firmware download and flashing utility;
- the corresponding modified source archives and license texts.

The release does **not** distribute the original Akko firmware or a prepatched full firmware image.

The firmware build script downloads the exact official firmware from the vendor firmware API, verifies its SHA256 checksum, applies the custom T75D Direct Mode patch locally, and verifies the resulting image.

> [!CAUTION]
> This is unofficial reverse-engineered software.
>
> Flashing firmware always carries a risk of making the device unusable.
> Do not use these files on any keyboard other than the exact supported model and firmware revision listed below.

## Supported device

This release supports only:

| Property | Required value |
|---|---|
| Keyboard | Akko TAC75 HE |
| USB VID:PID | `3151:502D` |
| Device/API ID | `2782` |
| Original firmware | `5.02` |
| Platform | Linux x86_64 |

Do not flash the generated firmware on another Akko or MonsGeek keyboard, even if it looks identical.

Windows is currently untested and unsupported.

## Requirements

The prebuilt release requires:

- Linux x86_64;
- an internet connection while building the firmware;
- a POSIX-compatible `/bin/sh`;
- Python 3.9 or newer;
- standard Linux tools including `sha256sum`, `awk` and `mktemp`;
- OpenRGB for using the included plugin.
- administrator access once, to install the included udev rules.

Rust, an ARM compiler and the complete development repository are not required.

## Release contents

```text
build-firmware.sh

patcher/
├── build_only.py
├── hook.bin
├── hook_framework.py
└── hooks.py

plugin/
└── libOpenRGBTAC75HEPlugin.so

tools/
└── iot_driver

udev/
└── 70-akko-tac75-he.rules

licenses/
├── iot_driver-GPL-3.0.txt
└── openrgb-plugin-GPL-2.0.txt

sources/
├── iot_driver-0.1.0-modified-source.tar.zst
├── OpenRGB-TAC75HE-Plugin-modified-source.tar.zst
└── TAC75HE-T75D-hook-source.tar.zst

README.md
SHA256SUMS
```

## Known limitations

- OpenRGB must be completely closed before using the Akko Web Driver.
- Do not use OpenRGB and the Akko Web Driver at the same time.
- The Caps Lock LED indicator does not work while OpenRGB Direct Mode is active.
- The prebuilt plugin may not be compatible with every OpenRGB or Qt build.
- Windows has not been tested.
- Only device ID `2782` with original firmware version `5.02` is supported.

Hall Effect settings are stored onboard and do not depend on OpenRGB. The normal
flashing process is expected to preserve them; see **Before installation**.

## Before installation

> [!IMPORTANT]
> The normal flashing process is designed to preserve the keyboard's onboard
> configuration.
>
> The generated firmware image does not overwrite the persistent flash area used
> by the stock firmware for stored device data. Existing key mappings, Hall
> Effect actuation settings, Rapid Trigger settings, deadzones and calibration
> are therefore expected to remain unchanged.
>
> This has only been tested on the supported TAC75 HE hardware and firmware
> revision. Before flashing, record or take screenshots of any important custom
> settings. After flashing, verify the key layout and Hall Effect behavior before
> normal use. Recalibrate the keyboard in the Akko Web Driver if any key behaves
> unexpectedly.

## Installation

> [!IMPORTANT]
> Install the included udev rules before flashing the keyboard.
>
> The TAC75 HE normally appears as USB device `3151:502D`. During firmware
> flashing it disconnects and reconnects as the Rongyuan bootloader device
> `3151:502A`.
>
> Without permission rules for both modes, the flashing utility may lose access
> to the keyboard after it enters the bootloader.

### 1. Verify the release files

Before running or installing anything, verify the release contents:

```bash
sha256sum -c SHA256SUMS
```

Do not continue if any file fails verification.

The generated firmware is not included in `SHA256SUMS` because it is built
locally. The build script verifies both the downloaded stock firmware and the
generated patched image separately.

### 2. Install the udev rules

Install the included rule:

```bash
sudo install -Dm644 \
  udev/70-akko-tac75-he.rules \
  /etc/udev/rules.d/70-akko-tac75-he.rules
```

Reload the udev configuration:

```bash
sudo udevadm control --reload-rules
```

Disconnect and reconnect the keyboard after installing the rule.

The rule grants the active local desktop user access only to:

- TAC75 HE normal mode: `3151:502D`;
- Rongyuan keyboard bootloader mode: `3151:502A`.

It does not grant access to every device using vendor ID `3151`.

### 3. Verify the keyboard

Check that the keyboard is connected using the supported USB VID:PID:

```bash
lsusb -d 3151:502d
```

Expected device:

```text
3151:502D
```

Do not continue if the keyboard is not detected with this exact VID:PID.

### 4. Build the patched firmware locally

Make the included scripts executable:

```bash
chmod +x build-firmware.sh tools/iot_driver
```

Build the firmware:

```bash
./build-firmware.sh
```

The script:

1. Downloads the official firmware for device ID `2782`.
2. Decompresses the vendor firmware package.
3. Verifies the exact stock firmware SHA256:
   `2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306`
4. Applies the T75D SAFE-GATE patch locally.
5. Verifies all patch locations, hook contents, image size and storage boundaries.
6. Verifies the generated firmware SHA256:
   `757c154ce58e656f68aca476daf68e76d1a7c4a40cc1d452b0b484ec4718e256`

The default output file is created in the current directory:

```text
TAC75_HE_2782_v502_T75D_SAFE_GATE.bin
```

A custom output path may be supplied as the first argument:

```bash
./build-firmware.sh /path/to/TAC75_HE_2782_v502_T75D_SAFE_GATE.bin
```

> [!NOTE]
> `build-firmware.sh` only downloads and builds files.
> It does not communicate with the keyboard and does not flash anything.

If the vendor API returns an unexpected firmware image, or any expected byte,
size or checksum differs, the build stops with an error.

### 5. Close conflicting software

Completely close OpenRGB before flashing:

```bash
pkill -x openrgb 2>/dev/null || true
```

Also close the Akko Web Driver.

OpenRGB and the Web Driver must not access the keyboard while firmware flashing
is in progress.

### 6. Flash the generated firmware

Flash only the locally generated and successfully verified image:

```bash
./tools/iot_driver firmware flash \
  TAC75_HE_2782_v502_T75D_SAFE_GATE.bin
```

Read the information printed by the utility.

When prompted, type:

```text
yes
```

Do not disconnect the keyboard, close the terminal or power off the computer
while flashing is in progress.

During flashing, the keyboard will reconnect as bootloader device `3151:502A`.
The included udev rule grants access to this mode.

After flashing finishes, disconnect and reconnect the keyboard.

### 7. Install the OpenRGB plugin

Install the plugin into the user OpenRGB plugin directory:

```bash
install -Dm755 \
  plugin/libOpenRGBTAC75HEPlugin.so \
  ~/.config/OpenRGB/plugins/libOpenRGBTAC75HEPlugin.so
```

Start OpenRGB normally.

The Akko TAC75 HE should appear as an RGB device.

## Normal usage

OpenRGB can control the keyboard lighting after the patched firmware and plugin are installed.

To change actuation, Rapid Trigger, deadzones, key mappings or other keyboard settings:

1. Close OpenRGB completely.
2. Disconnect and reconnect the keyboard if necessary.
3. Open the Akko Web Driver.
4. Read or change the keyboard settings.
5. Close the Akko Web Driver.
6. Start OpenRGB again.

OpenRGB and the Akko Web Driver access the same vendor HID interface. Running both at the same time may cause communication failures or freeze the Web Driver session.

## Uninstalling the OpenRGB plugin

```bash
rm -f ~/.config/OpenRGB/plugins/libOpenRGBTAC75HEPlugin.so
```

Removing the plugin disables OpenRGB integration but does not restore the original keyboard firmware.

## Restoring stock firmware

The included `iot_driver` can download the current firmware returned by the vendor API for device ID `2782`:

```bash
./tools/iot_driver firmware download \
  --device-id 2782 \
  --output TAC75_HE_2782_v502_stock.bin
```

Before flashing it, verify that it is the exact known stock firmware used by this release:

```bash
sha256sum TAC75_HE_2782_v502_stock.bin
```

The required result is:

```text
2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306
```

Do not flash the downloaded image if its size or checksum differs.

To restore the verified stock image:

```bash
./tools/iot_driver firmware flash \
  TAC75_HE_2782_v502_stock.bin
```

Read the information printed by the utility and type `yes` manually when prompted.

Never use firmware intended for another keyboard model, device ID or hardware revision.

## Firmware protocol

The patched firmware adds a custom Direct Mode protocol identified by the payload signature:

```text
T75D
```

Only signed T75D vendor packets are intercepted by the custom handler.

Unsigned original vendor commands are passed to the stock firmware, allowing the Akko Web Driver to remain functional while OpenRGB is closed.

While Direct Mode is active, the patched firmware prevents the stock lighting renderer from overwriting RGB frames sent by OpenRGB.

Leaving Direct Mode restores normal stock lighting behavior.

## Source code and licenses

This repository contains components distributed under different free software licenses. The repository root `LICENSE` file provides a component-by-component licensing notice.

The following components are distributed under **GPL-3.0-only**:

- `build-firmware.sh`;
- `patcher/`;
- `udev/`;
- `tools/iot_driver`;
- `sources/iot_driver-0.1.0-modified-source.tar.zst`;
- `sources/TAC75HE-T75D-hook-source.tar.zst`.

The complete GPL-3.0 license text is included at:

```text
licenses/iot_driver-GPL-3.0.txt
```

The modified `iot_driver` source archive corresponds to the prebuilt binary distributed as:

```text
tools/iot_driver
```

The T75D hook source archive contains the assembly sources, linker script, generated hook declarations, patch builder and verification code used to produce:

```text
patcher/hook.bin
```

The following components are distributed under **GPL-2.0-only**:

- `plugin/libOpenRGBTAC75HEPlugin.so`;
- `sources/OpenRGB-TAC75HE-Plugin-modified-source.tar.zst`.

The complete GPL-2.0 license text is included at:

```text
licenses/openrgb-plugin-GPL-2.0.txt
```

The OpenRGB plugin source archive corresponds to the prebuilt plugin included in this release.

The original Akko/Rongyuan firmware is not included in this repository or in the release archives. It is downloaded directly from the vendor firmware service by the user and is not covered by the licenses applied to this project's original code.

Third-party components and dependencies retain their respective licenses.

## Project status

This release should be considered an experimental alpha.

It has been tested on one keyboard with:

- Akko TAC75 HE;
- USB VID:PID `3151:502D`;
- device ID `2782`;
- original firmware version `5.02`;
- Linux x86_64;
- the included OpenRGB plugin.

No guarantees are made for other hardware revisions, firmware versions, operating systems or OpenRGB builds.

## Disclaimer

This project is not affiliated with or endorsed by Akko, MonsGeek, Rongyuan or OpenRGB.

The patcher, OpenRGB plugin and firmware utility are provided without warranty. You are responsible for verifying device compatibility and accepting all risks associated with firmware modification.
