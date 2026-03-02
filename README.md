# ESP32 433MHz Remote for Home Assistant

A Home Assistant integration that uses an ESP32-C6 Feather with a 433MHz RF transmitter to control a Minka Aire ceiling fan originally operated by a 433MHz remote.

## Hardware

- **Microcontroller**: Adafruit ESP32-C6 Feather (STEMMA QT) - [Product Page](https://www.adafruit.com/product/5933)
- **RF Transmitter**: Adafruit Radio FeatherWing - RFM69HCW 433MHz - [Product Page](https://www.adafruit.com/product/3230)

The FeatherWing plugs directly onto the ESP32-C6 Feather using the STEMMA QT connector.

### References

- [ESP32-C6 Feather I2C/SPI Guide](https://learn.adafruit.com/adafruit-esp32-c6-feather/i2c)
- [RFM69HCW 433MHz Library Documentation](https://learn.adafruit.com/adafruit-rfm69-hcw-915-israel/arduino-library)
- [ESPHome Documentation](https://esphome.io/)

## How It Works

1. The original Minka Aire remote signals were captured using a Flipper Zero Sub-GHz tool
2. These RF signal patterns are stored in `src/config.h`
3. When Home Assistant sends a command via MQTT, the ESP32 replays the captured RF signal
4. The ceiling fan responds to the RF signal just like the original remote

## Project Structure

```
.
├── platformio.ini        # PlatformIO build configuration
└── src/
    ├── main.cpp          # Application entry point
    ├── config.h          # WiFi, MQTT, RF signal configuration
    ├── secrets.example.h # Template for credentials (copy to secrets.h)
    ├── rfm69_driver.h    # RFM69 radio driver header
    ├── rfm69_driver.cpp  # RFM69 radio driver implementation
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
   ```

3. **Add your captured RF signals** in `src/config.h`:
   ```cpp
   // Replace these with your actual Flipper Zero captures
   const uint8_t SIGNAL_FAN_SPEED_1[] = {0x01, 0x00, 0x00, 0x00};
   ```

4. **Build and upload**:
   ```bash
   pio run
   pio run --target upload
   ```

5. **Monitor serial output**:
   ```bash
   pio device monitor
   ```

## Capturing RF Signals with Flipper Zero

### Step 1: Capture the Signal

1. Open Flipper Zero Sub-GHz menu
2. Select "Receive" mode
3. Press a button on your Minka Aire remote
4. Save the captured signal

### Step 2: Analyze the Signal

1. Export the capture as a `.sub` file
2. Use tools like [Flipper-SubGhz-Reader](https://github.com/G300B/Flipper-SubGhz-Reader) to decode
3. Extract the pulse pattern or raw data

### Step 3: Convert to Code

The captured signal needs to be converted to the format used by the RFM69 library. This typically involves:

- Converting pulse positions to binary data
- Identifying the modulation scheme (OOK, FSK, PPM, etc.)
- Creating the appropriate transmit sequence

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

Once the device is connected to WiFi, you can update firmware over-the-air:

```bash
pio run --target upload
```

## Troubleshooting

### Device not connecting to WiFi

- Check SSID and password in `src/config.h` or `src/secrets.h`
- Verify the ESP32 is within range of your router
- Check serial output for error messages

### Fan not responding to commands

- **RFM69 vs. OOK**: The RFM69 uses FSK modulation. Many 433MHz remotes (including Minka Aire) use OOK/ASK. If the fan does not respond, the RFM69 may be incompatible—consider an OOK-capable transmitter (e.g. SYN115, TX433).
- Verify the RF signal codes in `config.h` match your captured data
- Ensure the ESP32 is within range of the fan
- Try moving the ESP32 closer to the fan
- Check the RFM69 FeatherWing is properly seated

### MQTT connection fails

- Verify Home Assistant MQTT broker is running
- Check MQTT credentials in `src/config.h` or `src/secrets.h`
- Ensure firewall allows MQTT traffic (port 1883)

## License

This project is provided as-is for educational purposes. Ensure you have the right to control the devices you are integrating.

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.