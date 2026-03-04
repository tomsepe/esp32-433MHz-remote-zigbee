# Setup Guide: Minka Aire 433MHz Fan Remote

This guide walks you through hardware assembly, capturing RF signals with a Flipper Zero, programming the ESP32, and powering the device for permanent installation.

---

## Table of Contents

1. [Hardware Required](#1-hardware-required)
2. [Capturing RF Signals with Flipper Zero](#2-capturing-rf-signals-with-flipper-zero)
3. [Testing with Sub-GHz Remote App](#3-testing-with-sub-ghz-remote-app)
4. [Connecting the 433MHz Board to ESP32-C6](#4-connecting-the-433mhz-board-to-esp32-c6)
5. [Programming the ESP32-C6](#5-programming-the-esp32-c6)
6. [Power Requirements & 120VAC Setup](#6-power-requirements--120vac-setup)
7. [Software Configuration](#7-software-configuration)

---

## 1. Hardware Required

| Component | Description | Link |
|-----------|-------------|------|
| **ESP32-C6 Feather** | Microcontroller with WiFi, USB-C | [Adafruit Product](https://www.adafruit.com/product/5933) |
| **433 MHz OOK transmitter** | e.g. FS1000A — 433.92 MHz, DATA pin (OOK) | See `components/433MHz-Module/FS1000A-TX-RX-description.md` |
| **Flipper Zero** | For capturing remote signals (Read Raw → RAW_Data) | [Flipper Zero](https://flipperzero.one/) |
| **USB-C cable** | For programming and power during setup | — |
| **5V USB power supply** | 1A minimum (for permanent install) | — |

The Minka Aire remote uses **OOK at 433.92 MHz**. The FS1000A (or compatible) transmitter is driven by one GPIO: HIGH = carrier on, LOW = carrier off, with timings from your Flipper RAW capture.

---

## 2. Capturing RF Signals with Flipper Zero

Before the ESP32 can replay your fan’s RF commands, you must capture them from the original Minka Aire remote.

### Step 1: Find the Remote’s Frequency

1. Place the remote **very close** to the left side of the Flipper Zero.
2. Go to **Main Menu → Sub-GHz → Frequency Analyzer**.
3. **Press and hold** a button on the remote.
4. Note the frequency (often **433.92 MHz** for Minka Aire).

### Step 2: Set Up Read Mode

1. Go to **Main Menu → Sub-GHz → Read**.
2. Press **Config** to open the configuration menu.
3. Set **Frequency** to the value from Step 1 (e.g. 433.92 MHz).
4. Set **Modulation**:
   - **AM650** (default): typical for many 433 MHz remotes
   - **AM270** or **FM238** / **FM476** if AM650 fails

If the signal is weak or not decoded, try **Hopping: ON** to scan multiple frequencies.

### Step 3: Capture Each Button

1. Stay in **Read** mode with the correct frequency and modulation.
2. Press a button on the remote (e.g. Fan Low).
3. When a protocol is detected, press **OK** → **Save**.
4. Name the file (e.g. `fan_speed_1`) and save.
5. Repeat for every button: Fan Off, Speed 1/2/3, Light On/Off.

### Step 4: Read Raw (for OOK replay)

For this project we use **Read Raw** so we get exact timings to replay on the OOK transmitter:

1. Go to **Main Menu → Sub-GHz → Read Raw**.
2. Set the same frequency (433.92 MHz) and modulation (AM650) as above.
3. Press **Rec**, then press the remote button several times.
4. Press **Stop** → **Save** and name the file (e.g. `fan_power`).

Open the `.sub` file and copy the **RAW_Data** line into your firmware (see Software Configuration). Values are in microseconds: positive = carrier ON, negative = carrier OFF.

### References

- [Flipper Zero Sub-GHz Read](https://docs.flipper.net/zero/sub-ghz/read)
- [Flipper Zero Read Raw](https://docs.flipper.net/sub-ghz/read-raw)
- [Sub-GHz Remote app (community)](https://lab.flipper.net/apps/subghz_remote_ofw) — useful for testing saved signals

---

## 3. Testing with Sub-GHz Remote App

Use this to confirm captures work before programming the ESP32.

1. Install the **Sub-GHz Remote** app from [Flipper Lab](https://lab.flipper.net/apps/subghz_remote_ofw).
2. Go to **Sub-GHz → Saved** and select a captured signal.
3. Use **Emulate** or **Send** to replay it.
4. Check that the fan responds (speed change, light toggle, etc.).

If the fan does not respond:

- Retry with different modulation (AM650, AM270, FM238, FM476).
- Capture again with the remote closer to the Flipper Zero.
- Confirm you are within range of the fan during testing.

---

## 4. Connecting the 433 MHz OOK Transmitter to ESP32-C6

### Wiring (FS1000A or compatible)

| Transmitter pin | Connect to |
|-----------------|------------|
| **VCC**         | 3.3 V or 5 V (from Feather 3V or USB 5V) |
| **GND**         | GND |
| **DATA**        | One GPIO (e.g. GPIO 14); define in `src/config.h` |

Use short wires and keep the antenna (or 32 cm wire) away from metal. 3.3 V logic is fine; 5 V supply to the module can improve range.

### Pin choice

Pick any free GPIO and set it in `src/config.h` (e.g. `OOK_TX_GPIO 14`). Avoid pins used for USB, boot, or flash.

### Verify

1. Connect via USB-C, flash firmware, open serial monitor.
2. Trigger a command (e.g. from Home Assistant or a test in code); the fan should respond if the RAW_Data for that button is correct and the transmitter is powered and wired correctly.

---

## 5. Programming the ESP32-C6

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or standalone CLI)
- USB-C cable
- ESP32-C6 Feather + 433 MHz OOK transmitter (e.g. FS1000A) wired as above

### Steps

1. Open the project in VS Code (or your editor with PlatformIO).
2. Connect the stack via USB-C.
3. Build and upload (first flash must be over USB):

   ```bash
   pio run --target upload
   ```

4. Open the serial monitor (115200 baud). Note the device’s IP address printed at boot—you’ll need it for over-the-air updates:

   ```bash
   pio device monitor
   ```

**After the first successful USB flash**, you can update firmware over WiFi (OTA). Set `upload_port` in `platformio.ini` to the device’s IP and run `pio run --target upload`, or use `--upload-port=192.168.1.xxx`. See [README.md – OTA Updates](README.md#ota-updates) for details.

### Entering Bootloader Mode

If upload fails:

1. Hold the **Boot** button.
2. Press and release the **Reset** button.
3. Release the **Boot** button.
4. Run the upload again.

### Programming Interfaces

| Interface | Port | Use |
|-----------|------|-----|
| USB-C | Native | Programming, power, serial monitor |
| UART | TX=16, RX=17 | External serial adapter (optional) |

---

## 6. Power Requirements & 120VAC Setup

### Voltage Requirements

| Board / module | Input | Notes |
|----------------|-------|-------|
| ESP32-C6 Feather | 5 V (USB) or 3.7–4.2 V LiPo | Built-in 3.3 V regulator |
| FS1000A transmitter | 3–12 V (typ. 3.3 or 5 V) | DATA pin: 3.3 V logic from Feather |

Power the transmitter from 3.3 V or 5 V (e.g. Feather 3V or 5V pin). Do not backfeed 12 V into the Feather.

### Powering from 120VAC

For a permanent installation near the fan:

1. **USB wall adapter (recommended)**
   - Use a 5V, 1A (or higher) USB adapter.
   - Plug it into a 120V outlet.
   - Connect the stack with a USB-C cable.
   - Ensures stable 5V and isolates the ESP32 from mains.

2. **Alternative: AC–DC module**
   - Use a small 120VAC → 5VDC module (e.g. 5V 1A).
   - Connect its output to the Feather’s **USB** pin (5V) and **GND**.
   - Ensure correct polarity and isolation from mains.

### Placement Tips

- Mount the ESP32 and transmitter close to the fan for best RF link.
- Keep the antenna (or ~32 cm wire) away from metal that blocks 433 MHz.
- A small plastic enclosure with a USB power feed works well.

---

## 7. Software Configuration

After hardware is set up:

1. Copy `src/secrets.example.h` to `src/secrets.h`.
2. Edit `src/secrets.h` with your WiFi and MQTT settings. Optionally set `OTA_PASSWORD` for over-the-air updates (if set, add `upload_flags = --auth='your_password'` in `platformio.ini` when using OTA).
3. In `src/config.h`, add your Flipper **RAW_Data** timing arrays (one per button) from each `.sub` file. Use the same format: positive = ON µs, negative = OFF µs.
4. Set the OOK transmitter GPIO in `src/config.h` to match your wiring.
5. Build and upload again (first upload via USB; later you can use [OTA](README.md#ota-updates)).

See the main [README.md](README.md) for full configuration and Home Assistant integration details.

---

## Quick Reference

| Task | Command / Location |
|------|--------------------|
| Build | `pio run` |
| Upload (USB) | `pio run --target upload` |
| Upload (OTA) | `pio run --target upload` (set `upload_port` in `platformio.ini` to device IP) |
| Serial monitor | `pio device monitor` |
| Clean build | `pio run --target clean` |
| Credentials | `src/secrets.h` (optional: `OTA_PASSWORD` for OTA) |
| RF signal data | `src/config.h` |

---

## Troubleshooting

| Issue | Possible cause | Action |
|-------|----------------|--------|
| Fan not responding | Wrong RAW_Data or wiring | Confirm Flipper Emulate works; check DATA pin GPIO and VCC/GND; replay timing 2–3 times |
| No WiFi | Wrong SSID/password | Check `secrets.h` |
| No MQTT | Broker unreachable | Check IP, port 1883, credentials |
| Upload fails (USB) | Bootloader not active | Hold Boot, press Reset, retry upload |
| OTA upload fails | Wrong IP or auth | Set `upload_port` to device IP (see serial at boot); if using OTA password, set `upload_flags` in `platformio.ini` |

Ensure the `.sub` capture uses **Preset: FuriHalSubGhzPresetOok650Async** (OOK). The FS1000A is OOK-only and matches this.
