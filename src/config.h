/**
 * Configuration file for ESP32 433MHz Remote
 *
 * For credentials, use secrets.h (copy from secrets.example.h).
 * Fallback: define credentials here (do NOT commit to git).
 *
 * RF signals: paste RAW_Data from Flipper Zero .sub files (Read Raw).
 * Values are microseconds: positive = carrier ON, negative = carrier OFF.
 */

#ifndef CONFIG_H
#define CONFIG_H

// Include secrets if available (create from secrets.example.h)
#ifdef __has_include
  #if __has_include("secrets.h")
    #include "secrets.h"
  #endif
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

// ============================================================================
// DEVICE CONFIGURATION
// ============================================================================

const char* DEVICE_NAME = "minka-fan-remote";

// ============================================================================
// MQTT CONFIGURATION (Home Assistant)
// ============================================================================

#ifndef MQTT_SERVER
#define MQTT_SERVER "192.168.1.100"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef MQTT_USER
#define MQTT_USER ""
#endif
#ifndef MQTT_PASSWORD
#define MQTT_PASSWORD ""
#endif

// MQTT topic definitions are in ha_mqtt.h (command, state, config, availability)

// ============================================================================
// OOK TRANSMITTER (e.g. FS1000A)
// ============================================================================
//
// Connect transmitter DATA pin to this GPIO.
// VCC → 3.3V or 5V, GND → GND.
//

#define OOK_TX_GPIO  14

// ============================================================================
// RAW TIMING DATA (FROM FLIPPER ZERO .sub FILES)
// ============================================================================
//
// From each .sub file: Preset FuriHalSubGhzPresetOok650Async, Protocol: RAW.
// Copy the RAW_Data line; values are microseconds (positive = ON, negative = OFF).
// Use int16_t; length is in elements.
//

#define OOK_REPEAT_COUNT  2   // Send sequence this many times per command (like Flipper "Send")

// Placeholder: replace with your captured RAW_Data from each button.
// Example from power button capture (shortened); paste your full arrays.
static const int16_t SIGNAL_FAN_SPEED_1[] = {
    161, -264, 395, -132, 623, -134, 1335, -598, 65, -396
};
static const int16_t SIGNAL_FAN_SPEED_2[] = {
    161, -264, 395, -132, 623, -134, 1335, -598, 65, -396
};
static const int16_t SIGNAL_FAN_SPEED_3[] = {
    161, -264, 395, -132, 623, -134, 1335, -598, 65, -396
};
static const int16_t SIGNAL_FAN_OFF[] = {
    161, -264, 395, -132, 623, -134, 1335, -598, 65, -396
};
static const int16_t SIGNAL_LIGHT_ON[] = {
    161, -264, 395, -132, 623, -134, 1335, -598, 65, -396
};
static const int16_t SIGNAL_LIGHT_OFF[] = {
    161, -264, 395, -132, 623, -134, 1335, -598, 65, -396
};

#define SIGNAL_FAN_SPEED_1_LEN  (sizeof(SIGNAL_FAN_SPEED_1) / sizeof(SIGNAL_FAN_SPEED_1[0]))
#define SIGNAL_FAN_SPEED_2_LEN  (sizeof(SIGNAL_FAN_SPEED_2) / sizeof(SIGNAL_FAN_SPEED_2[0]))
#define SIGNAL_FAN_SPEED_3_LEN  (sizeof(SIGNAL_FAN_SPEED_3) / sizeof(SIGNAL_FAN_SPEED_3[0]))
#define SIGNAL_FAN_OFF_LEN      (sizeof(SIGNAL_FAN_OFF) / sizeof(SIGNAL_FAN_OFF[0]))
#define SIGNAL_LIGHT_ON_LEN     (sizeof(SIGNAL_LIGHT_ON) / sizeof(SIGNAL_LIGHT_ON[0]))
#define SIGNAL_LIGHT_OFF_LEN    (sizeof(SIGNAL_LIGHT_OFF) / sizeof(SIGNAL_LIGHT_OFF[0]))

#endif // CONFIG_H
