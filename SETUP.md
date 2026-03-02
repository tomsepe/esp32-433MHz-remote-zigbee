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
| **RFM69HCW 433MHz FeatherWing** | 433MHz RF transceiver | [Adafruit Product](https://www.adafruit.com/product/3230) |
| **Flipper Zero** | For capturing remote signals | [Flipper Zero](https://flipperzero.one/) |
| **USB-C cable** | For programming and power during setup | — |
| **5V USB power supply** | 1A minimum (for permanent install) | — |

The RFM69 FeatherWing stacks directly onto the ESP32-C6 Feather using the standard Feather header pins—no wires required for the radio connection.

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

### Step 4: Read Raw (Unknown Protocols)

If the remote uses a protocol Flipper Zero doesn’t recognize:

1. Go to **Main Menu → Sub-GHz → Read Raw**.
2. Set the same frequency and modulation as above.
3. Press **Rec** to start recording.
4. Press the remote button several times.
5. Press **Stop** → **Save** and name the file.

Raw captures need to be converted later to bytes for use in firmware.

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

## 4. Connecting the 433MHz Board to ESP32-C6

### Physical Assembly

1. Align the RFM69 FeatherWing with the ESP32-C6 Feather header pins.
2. Press the FeatherWing down firmly so all pins make contact.
3. No jumper wires are needed; the stack uses the Feather standard pinout.

### Pin Mapping (ESP32-C6 Feather)

| RFM69 Signal | ESP32-C6 GPIO | Function |
|--------------|---------------|----------|
| SCK          | 4             | SPI Clock |
| MISO         | 6             | SPI Data In |
| MOSI         | 5             | SPI Data Out |
| NSS (CS)     | 14            | Chip Select |
| IRQ          | 13            | Interrupt (optional) |
| RST          | 15            | Reset |

These are already defined in `src/config.h`. The FeatherWing receives **3.3V** and **GND** from the Feather.

### Verify Connection

1. Connect the stack via USB-C.
2. Flash the firmware (see next section).
3. Open the serial monitor—you should see `RFM69 initialized successfully at 433MHz`.

---

## 5. Programming the ESP32-C6

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or standalone CLI)
- USB-C cable
- ESP32-C6 + RFM69 stack

### Steps

1. Open the project in VS Code (or your editor with PlatformIO).
2. Connect the stack via USB-C.
3. Build and upload:

   ```bash
   pio run --target upload
   ```

4. Open the serial monitor (115200 baud):

   ```bash
   pio device monitor
   ```

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

| Board | Input | Notes |
|-------|-------|-------|
| ESP32-C6 Feather | 5V (USB) or 3.7–4.2V LiPo | Built-in 3.3V regulator |
| RFM69 FeatherWing | 3.3V | Powered from Feather |

The ESP32-C6 Feather needs **5V USB** (or LiPo) and supplies 3.3V to the FeatherWing.

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

- Mount the stack close to the fan for best RF link.
- Keep it away from metal enclosures that block 433 MHz.
- A small plastic enclosure with a USB power feed works well.

---

## 7. Software Configuration

After hardware is set up:

1. Copy `src/secrets.example.h` to `src/secrets.h`.
2. Edit `src/secrets.h` with your WiFi and MQTT settings.
3. Replace the placeholder RF data in `src/config.h` with your Flipper Zero captures.
4. Build and upload again.

See the main [README.md](README.md) for full configuration and Home Assistant integration details.

---

## Quick Reference

| Task | Command / Location |
|------|--------------------|
| Build | `pio run` |
| Upload | `pio run --target upload` |
| Serial monitor | `pio device monitor` |
| Clean build | `pio run --target clean` |
| Credentials | `src/secrets.h` |
| RF signal data | `src/config.h` |

---

## Troubleshooting

| Issue | Possible cause | Action |
|-------|----------------|--------|
| RFM69 init fails | Poor contact, wrong pins | Reseat FeatherWing, check pin mapping |
| Fan not responding | Wrong modulation or codes | Re-capture with different modulation, verify codes |
| No WiFi | Wrong SSID/password | Check `secrets.h` |
| No MQTT | Broker unreachable | Check IP, port 1883, credentials |
| Upload fails | Bootloader not active | Hold Boot, press Reset, retry upload |

**Modulation note:** The RFM69 uses FSK. Many 433 MHz remotes use OOK/ASK. If the fan never responds, the RFM69 may not be compatible; consider an OOK-capable transmitter instead.
