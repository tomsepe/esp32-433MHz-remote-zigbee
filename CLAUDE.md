# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-C6 Feather with RFM69HCW 433MHz radio controlling a Minka Aire ceiling fan via Home Assistant MQTT. RF signals captured using Flipper Zero Sub-GHz.

## Build & Development Commands

```bash
# Build project
pio run

# Upload via USB
pio run --target upload

# Monitor serial output
pio device monitor

# Clean build
pio run --target clean
```

## Architecture

- `src/main.cpp` - Entry point, WiFi/MQTT setup, command dispatch
- `src/config.h` - WiFi/MQTT settings (or use secrets.h), captured RF signal data
- `src/rfm69_driver.{h,cpp}` - RFM69HCW radio driver abstraction
- `src/ha_mqtt.{h,cpp}` - Home Assistant MQTT Discovery payloads

## Key Technical Details

- ESP32-C6 uses SPI pins: SCK=4, MISO=6, MOSI=5, CS=14
- RFM69 operates at 433MHz, 22dBm max power
- MQTT topics follow Home Assistant Discovery convention
- RF signal data in `config.h` must be replaced with actual Flipper Zero captures
- RFM69 uses FSK; many 433MHz remotes use OOK—may be incompatible with some fans

## Dependencies

- PlatformIO espressif32 platform
- Adafruit RFM69 library (via lib_deps)
- PubSubClient for MQTT
- ArduinoJson for JSON parsing