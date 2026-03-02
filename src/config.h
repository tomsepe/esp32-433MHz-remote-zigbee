/**
 * Configuration file for ESP32 433MHz Remote
 *
 * Edit this file with your network credentials and MQTT settings
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// WIFI CONFIGURATION
// ============================================================================

const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ============================================================================
// DEVICE CONFIGURATION
// ============================================================================

const char* DEVICE_NAME = "minka-fan-remote";

// ============================================================================
// MQTT CONFIGURATION (Home Assistant)
// ============================================================================

const char* MQTT_SERVER = "192.168.1.100";  // Home Assistant IP
const int MQTT_PORT = 1883;
const char* MQTT_USER = "";                  // Leave empty if no auth
const char* MQTT_PASSWORD = "";              // Leave empty if no auth

// MQTT Topics for Home Assistant discovery
#define MQTT_BASE_TOPIC "homeassistant"
#define MQTT_FAN_TOPIC "homeassistant/fan/minka_fan/config"
#define MQTT_LIGHT_TOPIC "homeassistant/light/minka_light/config"
#define MQTT_AVAILABILITY_TOPIC "homeassistant/fan/minka_fan/availability"

// ============================================================================
// RFM69HCW PIN DEFINITIONS (ESP32-C6 Feather STEMMA QT)
// ============================================================================
//
// The RFM69HCW FeatherWing connects via STEMMA QT connector:
//
// STEMMA QT Pin | ESP32-C6 GPIO | Function
// --------------|---------------|----------
// VCC           | 3.3V          | Power
// GND           | GND           | Ground
// SDA           | GPIO 8        | I2C SDA (not used by RFM69)
// SCL           | GPIO 7        | I2C SCL (not used by RFM69)
//
// RFM69HCW uses SPI interface:
//
// Signal        | ESP32-C6 GPIO | Description
// --------------|---------------|------------------
// SCK           | GPIO 4        | SPI Clock
// MISO          | GPIO 6        | SPI Master In Slave Out
// MOSI          | GPIO 5        | SPI Master Out Slave In
// NSS (CS)      | GPIO 14       | Chip Select
// IRQ           | GPIO 13       | Interrupt (optional)
// RST           | GPIO 15       | Reset pin
//
// ============================================================================

#define RFM69_SPI_SCK     4   // SPI Clock
#define RFM69_SPI_MISO    6   // SPI MISO
#define RFM69_SPI_MOSI    5   // SPI MOSI
#define RFM69_CS          14  // Chip Select
#define RFM69_IRQ         13  // Interrupt pin
#define RFM69_RST         15  // Reset pin

// ============================================================================
// RFM69 RF CONFIGURATION
// ============================================================================

#define RFM69_FREQUENCY   433.0   // MHz
#define RFM69_TX_POWER    22      // dBm (max for RFM69HCW)
#define RFM69_NODEID      1       // Unique node ID
#define RFM69_NETWORKID   100     // Network ID (must match on same network)

// ============================================================================
// CAPTURED RF SIGNALS (FROM FLIPPER ZERO)
// ============================================================================
//
// TODO: Replace these placeholder values with your actual captured signals
// from the Flipper Zero Sub-GHz capture.
//
// The Minka Aire remote uses pulse-position modulation (PPM) or similar.
// Use Flipper Zero to capture the actual signal patterns, then convert
// to the format expected by the RFM69 library.
//
// Example format (replace with actual values):
//   uint8_t FAN_SPEED_1[] = {0xAA, 0xBB, 0xCC, ...};
//
// ============================================================================

// Placeholder signal data - REPLACE WITH ACTUAL CAPTURED DATA
// These are dummy values that won't work with your fan
const uint8_t SIGNAL_FAN_SPEED_1[] = {0x01, 0x00, 0x00, 0x00};
const uint8_t SIGNAL_FAN_SPEED_2[] = {0x02, 0x00, 0x00, 0x00};
const uint8_t SIGNAL_FAN_SPEED_3[] = {0x03, 0x00, 0x00, 0x00};
const uint8_t SIGNAL_FAN_OFF[]     = {0x00, 0x00, 0x00, 0x00};
const uint8_t SIGNAL_LIGHT_ON[]    = {0x10, 0x00, 0x00, 0x00};
const uint8_t SIGNAL_LIGHT_OFF[]   = {0x11, 0x00, 0x00, 0x00};

// Signal lengths
#define SIGNAL_FAN_SPEED_1_LEN 4
#define SIGNAL_FAN_SPEED_2_LEN 4
#define SIGNAL_FAN_SPEED_3_LEN 4
#define SIGNAL_FAN_OFF_LEN     4
#define SIGNAL_LIGHT_ON_LEN    4
#define SIGNAL_LIGHT_OFF_LEN   4

#endif // CONFIG_H