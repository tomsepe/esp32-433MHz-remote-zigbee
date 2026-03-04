# ESP32 433MHz Remote for Home Assistant

A Home Assistant integration that uses an ESP32-C6 Feather with a 433MHz RF transmitter to control a Minka Aire ceiling fan originally operated by a 433MHz remote.

## Hardware

- **Microcontroller**: Adafruit ESP32-C6 Feather (STEMMA QT) - [Product Page](https://www.adafruit.com/product/5933)
- **RF Transmitter**: 433 MHz OOK transmitter module (e.g. FS1000A, XY-MK-5V) — 433.92 MHz, DATA pin driven by one GPIO

The transmitter uses **OOK (On-Off Keying)** to replay raw timing captures from a Flipper Zero. Connect the module’s DATA pin to a GPIO, plus VCC and GND. See [SETUP.md](SETUP.md) and `components/433MHz-Module/` for wiring and specs.

### References

- [ESP32-C6 Feather](https://learn.adafruit.com/adafruit-esp32-c6-feather)
- [Flipper Zero Sub-GHz](https://docs.flipper.net/zero/sub-ghz/read) — capturing OOK signals
- Project component: [FS1000A TX/RX description](components/433MHz-Module/FS1000A-TX-RX-description.md)

## How It Works

1. The original Minka Aire remote signals are captured with a Flipper Zero (Sub-GHz → Read Raw). The remote uses **OOK at 433.92 MHz**.
2. The **RAW_Data** timing sequence from each `.sub` file is stored in `src/config.h` (alternating ON/OFF durations in microseconds).
3. When Home Assistant sends a command via MQTT, the ESP32 drives the OOK transmitter’s DATA pin with that timing sequence.
4. The ceiling fan responds to the replayed OOK signal like the original remote.

## Project Structure

```
.
├── platformio.ini        # PlatformIO build configuration (includes OTA upload settings)
├── components/433MHz-Module/   # FS1000A TX/RX specs and wiring
├── arduino-example/      # Reference OOK TX/RX examples (renamed for clarity)
└── src/
    ├── main.cpp          # Entry point, WiFi/MQTT, ArduinoOTA, OOK timing replay
    ├── config.h          # WiFi, MQTT, OTA password, OOK GPIO and raw timing data
    ├── secrets.example.h # Template for credentials (copy to secrets.h)
    ├── ha_mqtt.h         # Home Assistant MQTT integration header
    └── ha_mqtt.cpp       # Home Assistant MQTT integration implementation
```

For detailed hardware assembly, Flipper Zero capture steps, and power setup, see **[SETUP.md](SETUP.md)**.

## Development Setup

### Prerequisites

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- ESP32 Arduino Core (auto-installed by PlatformIO)

### Quick Start

1. **Clone/Open the project** in PlatformIO

2. **Configure credentials** — either:
   - Copy `src/secrets.example.h` to `src/secrets.h` and fill in your values (recommended; `secrets.h` is gitignored), or
   - Edit the fallback values in `src/config.h`

   ```cpp
   // In secrets.h or config.h:
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASSWORD "your_wifi_password"
   #define MQTT_SERVER "192.168.1.100"
   #define MQTT_PORT 1883
   #define MQTT_USER ""
   #define MQTT_PASSWORD ""
   // Optional, for OTA updates: #define OTA_PASSWORD "your_ota_password"
   ```

3. **Add your captured RF signals** in `src/config.h`: paste the **RAW_Data** timing array from each Flipper `.sub` file (positive = carrier ON µs, negative = carrier OFF µs). See [SETUP.md](SETUP.md) for capture steps.

4. **Build and upload** (first time use USB; after that you can use [OTA](#ota-updates)):
   ```bash
   pio run
   pio run --target upload
   ```

5. **Monitor serial output** (device prints its IP at boot for later OTA):
   ```bash
   pio device monitor
   ```

## Capturing RF Signals with Flipper Zero

1. Use **Sub-GHz → Read Raw** with frequency **433.92 MHz** and modulation **AM650** (OOK). Press the remote button and save.
2. Confirm the `.sub` file shows **Preset: FuriHalSubGhzPresetOok650Async** and **Protocol: RAW**.
3. Test with **Emulate** / **Send** so the fan responds.
4. Copy the **RAW_Data** line from the `.sub` file into `src/config.h` as the timing array for that button (values in microseconds; positive = ON, negative = OFF).

See [SETUP.md](SETUP.md) for detailed capture steps and wiring.

## Home Assistant Integration

### MQTT Discovery

The device uses MQTT Discovery for automatic integration:

```yaml
# Home Assistant will automatically create:
# - fan.minka_aire_fan
# - light.minka_aire_light
```

### Manual Configuration

If automatic discovery doesn't work, add to `configuration.yaml`:

```yaml
fan:
  - platform: mqtt
    name: "Minka Aire Fan"
    command_topic: "homeassistant/fan/minka_fan/command"
    state_topic: "homeassistant/fan/minka_fan/state"
    speed_topic: "homeassistant/fan/minka_fan/speed"
    speeds:
      - "off"
      - "low"
      - "medium"
      - "high"

light:
  - platform: mqtt
    name: "Minka Aire Light"
    command_topic: "homeassistant/light/minka_light/command"
    state_topic: "homeassistant/light/minka_light/state"
```

## OTA Updates

You can update firmware over-the-air after the device is on WiFi, so you don’t need physical access once it’s installed.

1. **First flash must be over USB.** Flash the OTA-capable firmware once with:
   ```bash
   pio run --target upload
   ```
   (with the board connected by USB). The device will print its IP address at boot.

2. **Later updates over WiFi:** Set `upload_port` in `platformio.ini` to the device’s IP (e.g. `192.168.1.xxx`), or pass it when uploading:
   ```bash
   pio run --target upload --upload-port=192.168.1.xxx
   ```

3. **If you set an OTA password** in `src/secrets.h` (`OTA_PASSWORD`), add the same password to `platformio.ini` under `upload_flags`, e.g.:
   ```ini
   upload_flags = --auth='your_password'
   ```

See **[SETUP.md](SETUP.md)** for initial programming steps and optional OTA password in software configuration.

## Troubleshooting

### Device not connecting to WiFi

- Check SSID and password in `src/config.h` or `src/secrets.h`
- Verify the ESP32 is within range of your router
- Check serial output for error messages

### Fan not responding to commands

- Verify the **RAW_Data** timing in `config.h` matches your Flipper capture and that you’re replaying it multiple times (like the Flipper “Send”).
- Ensure the OOK transmitter DATA pin is on the correct GPIO and the module has power (3.3–5 V).
- Keep the transmitter close to the fan and away from metal that blocks 433 MHz.

### MQTT connection fails

- Verify Home Assistant MQTT broker is running
- Check MQTT credentials in `src/config.h` or `src/secrets.h`
- Ensure firewall allows MQTT traffic (port 1883)

### OTA upload fails

- Ensure the device is on WiFi and you have its current IP (printed at boot).
- Set `upload_port` in `platformio.ini` or use `pio run --target upload --upload-port=192.168.1.xxx`.
- If you use an OTA password, set `upload_flags = --auth='your_password'` in `platformio.ini` to match `OTA_PASSWORD` in `secrets.h`.

## License

This project is provided as-is for educational purposes. Ensure you have the right to control the devices you are integrating.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.