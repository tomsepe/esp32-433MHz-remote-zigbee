/**
 * ESP32 433MHz Remote for Home Assistant
 *
 * Controls a Minka Aire ceiling fan using 433MHz RF signals
 * Hardware: Adafruit ESP32-C6 Feather + RFM69HCW 433MHz FeatherWing
 * RF signals captured using Flipper Zero Sub-GHz
 */

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Include project-specific modules
#include "config.h"
#include "rfm69_driver.h"
#include "ha_mqtt.h"

// ============================================================================
// CONSTANTS
// ============================================================================

#define JSON_PAYLOAD_SIZE     256
#define MQTT_LOOP_DELAY_MS    50
#define WIFI_RETRY_INTERVAL_MS 30000  // Try WiFi reconnect every 30s
#define FAN_SPEED_MIN         1
#define FAN_SPEED_MAX         3

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
RFM69Driver rfm69;

bool wifiConnected = false;
bool mqttConnected = false;

// Track state for MQTT publishing (optimistic - we don't receive RF feedback)
int lastFanPercentage = 0;   // 0=off, 33=low, 66=med, 100=high
bool lastLightState = false;
unsigned long lastWifiRetry = 0;

// ============================================================================
// SETUP - Initialize all components
// ============================================================================

void setup() {
    // Start serial communication for debugging
    Serial.begin(115200);
    delay(100);  // Give serial time to initialize

    Serial.println("\n========================================");
    Serial.println("Minka Aire 433MHz Remote for Home Assistant");
    Serial.println("========================================");

    // Initialize the 433MHz RF transmitter
    Serial.println("\n[1/3] Initializing RFM69HCW 433MHz...");
    if (!rfm69.begin()) {
        Serial.println("ERROR: Failed to initialize RFM69!");
        while (1) delay(1000);  // Halt if radio fails
    }
    Serial.println("RFM69 initialized successfully");

    // Connect to WiFi network
    Serial.println("\n[2/3] Connecting to WiFi...");
    connectWiFi();
    if (!wifiConnected) {
        Serial.println("WARNING: WiFi connection failed, continuing without network");
    }

    // Initialize MQTT for Home Assistant integration
    Serial.println("\n[3/3] Initializing MQTT client...");
    setupMQTT();

    Serial.println("\n========================================");
    Serial.println("System ready!");
    Serial.println("========================================\n");
}

// ============================================================================
// MAIN LOOP - Handle incoming commands
// ============================================================================

void loop() {
    // Periodic WiFi reconnect when disconnected
    if (!wifiConnected && (millis() - lastWifiRetry > WIFI_RETRY_INTERVAL_MS)) {
        lastWifiRetry = millis();
        Serial.println("Retrying WiFi connection...");
        connectWiFi();
        if (wifiConnected && !mqttConnected) {
            setupMQTT();
        }
    }

    // Maintain MQTT connection
    if (mqttConnected) {
        mqttClient.loop();

        if (!mqttClient.connected()) {
            Serial.println("MQTT disconnected, reconnecting...");
            mqttConnected = false;
            setupMQTT();
        }
    }

    delay(MQTT_LOOP_DELAY_MS);
}

// ============================================================================
// WIFI CONNECTION
// ============================================================================

void connectWiFi() {
    Serial.print("Connecting to: ");
    Serial.println(WIFI_SSID);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setHostname(DEVICE_NAME);

    // Wait for connection with timeout
    int timeout = 30;  // 30 seconds
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(500);
        Serial.print(".");
        timeout--;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi connection failed!");
    }
}

// ============================================================================
// MQTT SETUP
// ============================================================================

void setupMQTT() {
    if (!wifiConnected) {
        Serial.println("Cannot setup MQTT without WiFi");
        return;
    }

    // Configure MQTT client
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    // Generate unique client ID from MAC (ESP.getChipId deprecated on ESP32-C6)
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char clientIdBuf[40];
    snprintf(clientIdBuf, sizeof(clientIdBuf), "%s_%02x%02x%02x%02x%02x%02x",
             DEVICE_NAME, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Connect to MQTT broker
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT...");

        if (mqttClient.connect(clientIdBuf, MQTT_USER, MQTT_PASSWORD)) {
            mqttConnected = true;
            Serial.println("Connected!");

            // Subscribe to Home Assistant command topics
            mqttClient.subscribe(MQTT_FAN_TOPIC);
            mqttClient.subscribe(HA_FAN_PERCENTAGE_TOPIC);
            mqttClient.subscribe(MQTT_LIGHT_TOPIC);

            // Announce device and publish discovery
            announceDevice();
        } else {
            Serial.print("Failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" Retrying in 5s");
            delay(5000);
        }
    }
}

// ============================================================================
// ANNOUNCE DEVICE TO HOME ASSISTANT
// ============================================================================

void announceDevice() {
    // Publish MQTT Discovery payloads for automatic HA integration
    HADiscovery::publishFanConfig(mqttClient);
    HADiscovery::publishLightConfig(mqttClient);
    HADiscovery::publishAvailability(mqttClient, "online");
    Serial.println("Device announced to Home Assistant");
}

// ============================================================================
// MQTT CALLBACK - Handle incoming messages
// ============================================================================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT Topic: ");
    Serial.println(topic);

    // Handle fan percentage topic (raw payload: "0"-"100")
    if (strcmp(topic, HA_FAN_PERCENTAGE_TOPIC) == 0) {
        char buf[8];
        size_t copyLen = (length < sizeof(buf) - 1) ? length : sizeof(buf) - 1;
        memcpy(buf, payload, copyLen);
        buf[copyLen] = '\0';
        int pct = atoi(buf);
        int speed = (pct <= 0) ? 0 : (pct <= 33) ? 1 : (pct <= 66) ? 2 : 3;
        if (speed == 0) {
            transmitFanOff();
        } else {
            transmitFanSpeed(speed);
        }
        return;
    }

    // Handle fan command topic (JSON or raw "ON"/"OFF")
    if (strcmp(topic, MQTT_FAN_TOPIC) == 0) {
        if (length <= 4) {
            // Raw payload: ON, OFF
            char buf[8];
            memcpy(buf, payload, length);
            buf[length] = '\0';
            if (strcmp(buf, "ON") == 0 || strcmp(buf, "on") == 0) {
                transmitFanSpeed(1);
                return;
            }
            if (strcmp(buf, "OFF") == 0 || strcmp(buf, "off") == 0) {
                transmitFanOff();
                return;
            }
        }
        StaticJsonDocument<JSON_PAYLOAD_SIZE> doc;
        if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
            const char* command = doc["command"];
            if (command) {
                handleFanCommand(command, doc);
            }
        }
        return;
    }

    // Handle light command topic (JSON or raw "ON"/"OFF")
    if (strcmp(topic, MQTT_LIGHT_TOPIC) == 0) {
        if (length <= 4) {
            char buf[8];
            memcpy(buf, payload, length);
            buf[length] = '\0';
            if (strcmp(buf, "ON") == 0 || strcmp(buf, "on") == 0) {
                transmitLightOn();
                return;
            }
            if (strcmp(buf, "OFF") == 0 || strcmp(buf, "off") == 0) {
                transmitLightOff();
                return;
            }
        }
        StaticJsonDocument<JSON_PAYLOAD_SIZE> doc;
        if (deserializeJson(doc, payload, length) == DeserializationError::Ok) {
            const char* command = doc["command"];
            if (command) {
                handleLightCommand(command);
            }
        }
    }
}

// ============================================================================
// FAN COMMAND HANDLER
// ============================================================================

void handleFanCommand(const char* command, JsonDocument& doc) {
    Serial.print("Fan command: ");
    Serial.println(command);

    // Handle different fan commands
    if (strcmp(command, "on") == 0 || strcmp(command, "start") == 0) {
        int speed = doc["speed"] | 1;  // Default to speed 1
        transmitFanSpeed(speed);
    }
    else if (strcmp(command, "off") == 0 || strcmp(command, "stop") == 0) {
        transmitFanOff();
    }
    else if (strcmp(command, "speed") == 0) {
        int speed = doc["speed"];
        transmitFanSpeed(speed);
    }
}

// ============================================================================
// LIGHT COMMAND HANDLER
// ============================================================================

void handleLightCommand(const char* command) {
    Serial.print("Light command: ");
    Serial.println(command);

    if (strcmp(command, "on") == 0) {
        transmitLightOn();
    }
    else if (strcmp(command, "off") == 0) {
        transmitLightOff();
    }
}

// ============================================================================
// RF TRANSMISSION WRAPPERS
// ============================================================================

void transmitFanSpeed(int speed) {
    // Clamp to valid range; driver will reject invalid values
    int clamped = (speed < FAN_SPEED_MIN) ? FAN_SPEED_MIN
                : (speed > FAN_SPEED_MAX) ? FAN_SPEED_MAX : speed;
    int pct = (clamped == 1) ? 33 : (clamped == 2) ? 66 : 100;
    Serial.print("Transmitting fan speed ");
    Serial.println(clamped);
    rfm69.transmitSignal(clamped);
    lastFanPercentage = pct;
    if (mqttConnected) {
        HADiscovery::publishFanState(mqttClient, "on", pct);
    }
}

void transmitFanOff() {
    Serial.println("Transmitting fan off");
    rfm69.transmitFanOff();
    lastFanPercentage = 0;
    if (mqttConnected) {
        HADiscovery::publishFanState(mqttClient, "off", 0);
    }
}

void transmitLightOn() {
    Serial.println("Transmitting light on");
    rfm69.transmitLightOn();
    lastLightState = true;
    if (mqttConnected) {
        HADiscovery::publishLightState(mqttClient, "on");
    }
}

void transmitLightOff() {
    Serial.println("Transmitting light off");
    rfm69.transmitLightOff();
    lastLightState = false;
    if (mqttConnected) {
        HADiscovery::publishLightState(mqttClient, "off");
    }
}