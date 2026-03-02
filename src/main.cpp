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
// GLOBAL VARIABLES
// ============================================================================

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
RFM69Driver rfm69;

bool wifiConnected = false;
bool mqttConnected = false;

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
    // Maintain MQTT connection
    if (mqttConnected) {
        mqttClient.loop();

        // Reconnect if disconnected
        if (!mqttClient.connected()) {
            Serial.println("MQTT disconnected, reconnecting...");
            setupMQTT();
        }
    }

    // Small delay to prevent busy-waiting
    delay(50);
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

    // Generate unique client ID
    String clientId = String(DEVICE_NAME) + "_" + String(ESP.getChipId(), HEX);

    // Connect to MQTT broker
    while (!mqttClient.connected()) {
        Serial.print("Connecting to MQTT...");

        if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
            mqttConnected = true;
            Serial.println("Connected!");

            // Subscribe to Home Assistant command topics
            mqttClient.subscribe(MQTT_FAN_TOPIC);
            mqttClient.subscribe(MQTT_LIGHT_TOPIC);

            // Announce device presence
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
    // Send device availability status
    mqttClient.publish(MQTT_AVAILABILITY_TOPIC, "online", true);
    Serial.println("Device announced to Home Assistant");
}

// ============================================================================
// MQTT CALLBACK - Handle incoming messages
// ============================================================================

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("MQTT Topic: ");
    Serial.println(topic);

    // Parse JSON payload
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }

    // Handle fan commands
    if (strcmp(topic, MQTT_FAN_TOPIC) == 0) {
        const char* command = doc["command"];
        if (command) {
            handleFanCommand(command, doc);
        }
    }
    // Handle light commands
    else if (strcmp(topic, MQTT_LIGHT_TOPIC) == 0) {
        const char* command = doc["command"];
        if (command) {
            handleLightCommand(command);
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
    Serial.print("Transmitting fan speed ");
    Serial.println(speed);
    rfm69.transmitSignal(speed);
}

void transmitFanOff() {
    Serial.println("Transmitting fan off");
    rfm69.transmitFanOff();
}

void transmitLightOn() {
    Serial.println("Transmitting light on");
    rfm69.transmitLightOn();
}

void transmitLightOff() {
    Serial.println("Transmitting light off");
    rfm69.transmitLightOff();
}