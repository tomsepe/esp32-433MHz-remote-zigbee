/**
 * ESP32 433MHz Remote for Home Assistant
 *
 * Controls a Minka Aire ceiling fan using 433 MHz OOK (On-Off Keying).
 * Hardware: ESP32-C6 Feather + OOK transmitter (e.g. FS1000A) on one GPIO.
 * RF signals: Flipper Zero Sub-GHz Read Raw → RAW_Data in config.h.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "ha_mqtt.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

void connectWiFi();
void setupMQTT();
void announceDevice();
void transmitFanOff();
void transmitFanSpeed(int speed);
void transmitLightOn();
void transmitLightOff();
void handleFanCommand(const char* command, JsonDocument& doc);
void handleLightCommand(const char* command);

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

    // OOK transmitter DATA pin
    Serial.println("\n[1/4] Initializing 433 MHz OOK transmitter...");
    pinMode(OOK_TX_GPIO, OUTPUT);
    digitalWrite(OOK_TX_GPIO, LOW);
    Serial.printf("OOK transmitter ready (DATA on GPIO %d)\n", OOK_TX_GPIO);

    // Connect to WiFi network
    Serial.println("\n[2/4] Connecting to WiFi...");
    connectWiFi();
    if (!wifiConnected) {
        Serial.println("WARNING: WiFi connection failed, continuing without network");
    }

    // Over-the-air updates (only when WiFi is up)
    if (wifiConnected) {
        Serial.println("\n[3/4] Starting ArduinoOTA...");
        ArduinoOTA.setHostname(DEVICE_NAME);
#ifdef OTA_PASSWORD
        ArduinoOTA.setPassword(OTA_PASSWORD);
#endif
        ArduinoOTA.onStart([]() {
            Serial.println("OTA update start");
        });
        ArduinoOTA.onEnd([]() {
            Serial.println("\nOTA update end");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("OTA progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t err) {
            Serial.printf("OTA error %u: ", err);
            if (err == OTA_AUTH_ERROR) Serial.println("auth failed");
            else if (err == OTA_BEGIN_ERROR) Serial.println("begin failed");
            else if (err == OTA_CONNECT_ERROR) Serial.println("connect failed");
            else if (err == OTA_RECEIVE_ERROR) Serial.println("receive failed");
            else if (err == OTA_END_ERROR) Serial.println("end failed");
            else Serial.println("unknown");
        });
        ArduinoOTA.begin();
        Serial.println("ArduinoOTA ready");
    }

    // Initialize MQTT for Home Assistant integration
    Serial.println("\n[4/4] Initializing MQTT client...");
    setupMQTT();

    Serial.println("\n========================================");
    Serial.println("System ready!");
    Serial.println("========================================\n");
}

// ============================================================================
// MAIN LOOP - Handle incoming commands
// ============================================================================

void loop() {
    ArduinoOTA.handle();

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
// OOK RAW TIMING REPLAY
// ============================================================================

/**
 * Replay Flipper RAW_Data timing on the OOK transmitter DATA pin.
 * Positive value = carrier ON for that many µs, negative = OFF.
 */
void sendOOKRaw(const int16_t* timings, size_t count, uint8_t repeats) {
    for (uint8_t r = 0; r < repeats; r++) {
        for (size_t i = 0; i < count; i++) {
            int16_t t = timings[i];
            if (t > 0) {
                digitalWrite(OOK_TX_GPIO, HIGH);
                delayMicroseconds((unsigned int)t);
            } else if (t < 0) {
                digitalWrite(OOK_TX_GPIO, LOW);
                delayMicroseconds((unsigned int)(-t));
            }
        }
        if (r + 1 < repeats) {
            delay(10);
        }
    }
    digitalWrite(OOK_TX_GPIO, LOW);
}

// ============================================================================
// RF TRANSMISSION WRAPPERS
// ============================================================================

void transmitFanSpeed(int speed) {
    int clamped = (speed < FAN_SPEED_MIN) ? FAN_SPEED_MIN
                : (speed > FAN_SPEED_MAX) ? FAN_SPEED_MAX : speed;
    int pct = (clamped == 1) ? 33 : (clamped == 2) ? 66 : 100;
    Serial.print("Transmitting fan speed ");
    Serial.println(clamped);

    const int16_t* sig = nullptr;
    size_t len = 0;
    switch (clamped) {
        case 1: sig = SIGNAL_FAN_SPEED_1; len = SIGNAL_FAN_SPEED_1_LEN; break;
        case 2: sig = SIGNAL_FAN_SPEED_2; len = SIGNAL_FAN_SPEED_2_LEN; break;
        case 3: sig = SIGNAL_FAN_SPEED_3; len = SIGNAL_FAN_SPEED_3_LEN; break;
        default: return;
    }
    sendOOKRaw(sig, len, OOK_REPEAT_COUNT);

    lastFanPercentage = pct;
    if (mqttConnected) {
        HADiscovery::publishFanState(mqttClient, "on", pct);
    }
}

void transmitFanOff() {
    Serial.println("Transmitting fan off");
    sendOOKRaw(SIGNAL_FAN_OFF, SIGNAL_FAN_OFF_LEN, OOK_REPEAT_COUNT);
    lastFanPercentage = 0;
    if (mqttConnected) {
        HADiscovery::publishFanState(mqttClient, "off", 0);
    }
}

void transmitLightOn() {
    Serial.println("Transmitting light on");
    sendOOKRaw(SIGNAL_LIGHT_ON, SIGNAL_LIGHT_ON_LEN, OOK_REPEAT_COUNT);
    lastLightState = true;
    if (mqttConnected) {
        HADiscovery::publishLightState(mqttClient, "on");
    }
}

void transmitLightOff() {
    Serial.println("Transmitting light off");
    sendOOKRaw(SIGNAL_LIGHT_OFF, SIGNAL_LIGHT_OFF_LEN, OOK_REPEAT_COUNT);
    lastLightState = false;
    if (mqttConnected) {
        HADiscovery::publishLightState(mqttClient, "off");
    }
}