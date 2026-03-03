# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ESP32-C6 Feather with a 433 MHz OOK transmitter (e.g. FS1000A) controlling a Minka Aire ceiling fan via Home Assistant MQTT. RF signals are captured with Flipper Zero (Sub-GHz Read Raw); RAW_Data timing is replayed on the transmitter DATA pin.

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
- `src/config.h` - WiFi/MQTT, OOK TX GPIO, RAW_Data timing arrays from Flipper .sub
- `src/rfm69_driver.{h,cpp}` - RF driver: replays OOK timing on one GPIO (DATA pin of FS1000A)
- `src/ha_mqtt.{h,cpp}` - Home Assistant MQTT Discovery payloads

## Key Technical Details

- OOK transmitter (e.g. FS1000A): one GPIO → DATA pin; HIGH = carrier on, LOW = off. Timings from Flipper RAW_Data (µs).
- Minka Aire remote: OOK at 433.92 MHz (Preset FuriHalSubGhzPresetOok650Async).
- MQTT topics follow Home Assistant Discovery convention.
- RAW_Data in config: positive = ON µs, negative = OFF µs; replay 2–3 times per command.

## Dependencies

- PlatformIO espressif32 platform
- PubSubClient for MQTT
- ArduinoJson for JSON parsing