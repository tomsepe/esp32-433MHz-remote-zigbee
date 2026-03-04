/**
 * Secrets template for ESP32 433MHz Remote
 *
 * Copy this file to secrets.h (same directory) and fill in your values.
 * DO NOT commit secrets.h to version control!
 *
 * Add to .gitignore: src/secrets.h
 */

#ifndef SECRETS_EXAMPLE_H
#define SECRETS_EXAMPLE_H

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_SERVER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""

// Optional: set a password for OTA firmware updates (ArduinoOTA).
// If set, add upload_flags = --auth='your_password' in platformio.ini.
#define OTA_PASSWORD ""

#endif
