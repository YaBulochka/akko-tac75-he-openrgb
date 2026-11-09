# Akko TAC75 HE OpenRGB Support

Unofficial and experimental OpenRGB Direct Mode support for the **Akko TAC75 HE**.

This repository contains:

- a local firmware builder that downloads the official stock firmware, verifies it and applies the T75D patch;
- a Linux OpenRGB plugin and a Windows build workflow;
- a firmware utility and a safety wrapper for flashing;
- source archives and license texts.

It does **not** distribute the original Akko firmware or a prepatched firmware image.

> [!CAUTION]
> This is unofficial reverse-engineered software. Flashing firmware can make the
> keyboard unusable. Use it only with the exact model and firmware revision below.

## Supported device

| Property | Required value |
|---|---|
| Keyboard | Akko TAC75 HE |
| USB VID:PID | `3151:502D` |
| Device/API ID | `2782` |
| Original firmware | `5.02` |
| Platform | Linux x86_64, glibc 2.39 or newer |
| Prebuilt plugin | OpenRGB 1.1, Qt 6.12 |

Do not flash the generated image on another Akko or MonsGeek keyboard, even if it
looks identical.

## Requirements

- Linux x86_64 with glibc 2.39 or newer.
- An internet connection while building the firmware.
- A POSIX-compatible `/bin/sh`, Python 3.9 or newer, `sha256sum`, `awk`,
  `mktemp` and `lsusb`.
- Runtime libraries for `iot_driver`: `libudev.so.1`, `libssl.so.3` and
  `libasound.so.2`.
- OpenRGB 1.1 built with Qt 6.12 for the prebuilt plugin, or a locally rebuilt
  plugin.
- Administrator access once, to install the udev rule.

The prebuilt `iot_driver` is built on a recent Arch Linux system. On an older
distribution, rebuild it from `sources/iot_driver-0.1.0-modified-source.tar.zst`.

The bundled `iot_driver` is a shared utility. Its help text still mentions
MonsGeek. TAC75 HE support is provided by this release.

## Known limitations

- OpenRGB must be completely closed before using the Akko Web Driver.
- Do not run OpenRGB and the Akko Web Driver at the same time.
- Caps, Scroll and WinLock indicator LEDs are copied from the stock frame
  while Direct Mode is active. This behavior is new and has not been verified
  on hardware yet.
- If OpenRGB is killed while Direct Mode is active, the keyboard may stay in
  Direct Mode until it is disconnected and reconnected.
- The plugin probes the keyboard only when it is loaded. If you connect the
  keyboard after OpenRGB starts, restart OpenRGB or disable and re-enable the
  plugin.
- Only device ID `2782` with original firmware `5.02` is supported.
- The Windows plugin has not been tested on Windows.

## Before installation

The patch only writes the application area and is designed to preserve the
persistent flash area used for key mappings, Hall Effect settings, Rapid Trigger,
deadzones and calibration.

This has only been tested on the supported hardware and firmware. Before
flashing, record or take screenshots of important settings. After flashing,
verify the key layout and Hall Effect behavior. Recalibrate in the Akko Web
Driver if a key behaves unexpectedly.

## Installation

### 1. Verify the release files

```bash
sha256sum -c SHA256SUMS
```

Do not continue if any file fails verification. The generated firmware is not in
`SHA256SUMS` because it is built locally and verified by the build script.

### 2. Install the udev rules

```bash
sudo install -Dm644 \
  udev/70-akko-tac75-he.rules \
  /etc/udev/rules.d/70-akko-tac75-he.rules
sudo udevadm control --reload-rules
```

Disconnect and reconnect the keyboard after installing the rule. The rule grants
the active local user access to `3151:502D` and the bootloader `3151:502A` only.

### 3. Verify the keyboard

```bash
lsusb -d 3151:502d
```

Expected output contains `3151:502D`. Do not continue if the keyboard is not
detected with this exact VID:PID.

### 4. Build the patched firmware locally

```bash
chmod +x build-firmware.sh tools/flash-tac75he.sh tools/iot_driver
./build-firmware.sh
```

The script downloads the official firmware for device ID `2782`, verifies the
stock SHA256 `2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306`,
applies the T75D SAFE-GATE patch and verifies the output SHA256
`d067d58a24bb9dfcd5bf1f0c685aa8f320e5e64b880f865979a38d781d0e9b8e`.

The default output is created in the current directory:

```text
TAC75_HE_2782_v502_T75D_SAFE_GATE.bin
```

A custom output path may be supplied as the first argument:

```bash
./build-firmware.sh /path/to/TAC75_HE_2782_v502_T75D_SAFE_GATE.bin
```

`build-firmware.sh` only builds the image. It does not communicate with the
keyboard and does not flash anything.

### 5. Close conflicting software

```bash
pkill -x openrgb 2>/dev/null || true
```

Also close the Akko Web Driver. OpenRGB and the Web Driver must not access the
keyboard while flashing.

### 6. Flash the generated firmware

```bash
./tools/flash-tac75he.sh \
  TAC75_HE_2782_v502_T75D_SAFE_GATE.bin
```

The wrapper verifies the image SHA256, the USB VID:PID, device ID `2782` and
firmware version `5.02` before flashing. It asks for confirmation; type `yes`.

Do not disconnect the keyboard, close the terminal or power off the computer
while flashing. The keyboard reconnects as bootloader `3151:502A`; the udev rule
grants access to that mode. Reconnect the keyboard after flashing finishes.

> [!NOTE]
> The wrapper refuses to run when the keyboard is already in bootloader mode.
> Reconnect it in normal mode first. In bootloader mode the model cannot be
> verified, so recovery requires the raw `iot_driver firmware flash <file>`
> command and extra care.

### 7. Install the Linux plugin

```bash
install -Dm755 \
  plugin/bin/linux/libOpenRGBTAC75HEPlugin.so \
  ~/.config/OpenRGB/plugins/libOpenRGBTAC75HEPlugin.so
```

Start OpenRGB. The Akko TAC75 HE should appear as an RGB device.

## Windows plugin

The Windows plugin has **not been tested on Windows**, and this repository does
not include a prebuilt Windows DLL. The `Build Windows Plugin` GitHub Actions
workflow builds an x86_64 DLL with Qt 6.8.3, MSVC 2022 and `hidapi-hotplug.dll`,
matching the official OpenRGB 1.1 Windows build. The artifact contains
`OpenRGBTAC75HEPlugin.dll` and `hidapi-hotplug.dll`.

To install the artifact:

1. Open the latest successful `Build Windows Plugin` run under **Actions** and
   download the `OpenRGBTAC75HEPlugin-windows-x86_64-qt6.8.3` artifact.
2. Copy `OpenRGBTAC75HEPlugin.dll` to `%APPDATA%\OpenRGB\plugins\`.
3. Make sure `hidapi-hotplug.dll` is next to `OpenRGB.exe`. OpenRGB 1.1 already
   ships it; if it is missing, copy the DLL from the artifact into the OpenRGB
   program directory.
4. Start OpenRGB.

The plugin uses the MSVC 2022 runtime. Install the Microsoft Visual C++
2015-2022 Redistributable (x64) if Windows reports a missing runtime DLL.

The firmware build and flashing steps in this README are Linux-only.

## Building the Linux plugin from source

Requirements:

- `qmake6` with Qt 6 Core, Gui and Widgets development packages;
- `hidapi` with the `hidapi-hidraw` pkg-config module;
- `g++`, `make` and `pkg-config`.

Build:

```bash
cd plugin/src/linux
./build.sh
```

The result is written to `plugin/bin/linux/libOpenRGBTAC75HEPlugin.so`. The
script prefers the Qt major version used by `openrgb` in `PATH`. The prebuilt
plugin requires Qt 6.12; rebuild it for a different Qt version.

The vendored OpenRGB plugin headers can be refreshed from an upstream tag:

```bash
cd plugin/src/linux
./setup-openrgb.sh release_1.1
```

## Normal usage

OpenRGB can control the keyboard lighting after the patched firmware and plugin
are installed.

To change actuation, Rapid Trigger, deadzones, key mappings or other settings:

1. Close OpenRGB completely.
2. Disconnect and reconnect the keyboard if necessary.
3. Open the Akko Web Driver.
4. Read or change the settings.
5. Close the Akko Web Driver.
6. Start OpenRGB again.

OpenRGB and the Akko Web Driver access the same vendor HID interface. Running
both at the same time may cause communication failures or freeze the Web Driver
session.

## Uninstalling the plugin

```bash
rm -f ~/.config/OpenRGB/plugins/libOpenRGBTAC75HEPlugin.so
```

Removing the plugin disables OpenRGB integration but does not restore the
original keyboard firmware.

## Restoring stock firmware

Download the current vendor firmware for device ID `2782`:

```bash
./tools/iot_driver firmware download \
  --device-id 2782 \
  --output TAC75_HE_2782_v502_stock.bin
```

Flash it through the safety wrapper with `--stock`:

```bash
./tools/flash-tac75he.sh --stock \
  TAC75_HE_2782_v502_stock.bin
```

The wrapper verifies the known stock SHA256
`2e188151b6025aac685855c6088959fa3d9c9058d9055bfb07a92c380b68f306`, the USB
VID:PID, device ID `2782` and firmware `5.02` before flashing.

Never use firmware intended for another keyboard model, device ID or hardware
revision.

## Firmware protocol

The patched firmware adds a Direct Mode protocol with the payload signature
`T75D`. Only signed T75D vendor packets are intercepted; unsigned stock commands
are passed to the original firmware, so the Akko Web Driver still works while
OpenRGB is closed. While Direct Mode is active, the stock lighting renderer
cannot overwrite OpenRGB frames. Leaving Direct Mode restores stock lighting.

## Licenses

- GPL-3.0-only: `build-firmware.sh`, `patcher/`, `scripts/`, `tests/`, `udev/`
  and `tools/`.
- GPL-2.0-only: `plugin/` and the prebuilt Linux plugin binary.
- Full license texts: `licenses/`.
- Component-by-component notice: root `LICENSE`.

Vendored third-party files under `plugin/src/` retain their own licenses. The
original Akko/Rongyuan firmware is not included; it is downloaded by the user.

## Project status

This is an experimental alpha. It has been tested on one keyboard:

- Akko TAC75 HE, USB `3151:502D`, device ID `2782`, firmware `5.02`;
- Linux x86_64;
- the included Linux OpenRGB plugin.

The Windows plugin is built by CI but has not been tested on Windows. No
guarantees are made for other hardware revisions, firmware versions, operating
systems or OpenRGB builds.

## Disclaimer

This project is not affiliated with or endorsed by Akko, MonsGeek, Rongyuan or
OpenRGB. The patcher, plugin and firmware utility are provided without warranty.
You are responsible for verifying device compatibility and accepting all risks
associated with firmware modification.
