/**
 * Home Assistant MQTT Discovery Implementation
 *
 * Publishes MQTT Discovery payloads for automatic device registration
 */

#include "ha_mqtt.h"
#include "config.h"
#include <ArduinoJson.h>

// ============================================================================
// FAN DISCOVERY
// ============================================================================

void HADiscovery::publishFanConfig(PubSubClient& client) {
    StaticJsonDocument<512> doc;

    // Device information
    JsonObject device = doc.createNestedObject("device");
    device["name"] = "Minka Aire Fan";
    device["identifiers"] = DEVICE_NAME;
    device["manufacturer"] = "Minka Aire";
    device["model"] = "433MHz Remote Control";
    device["sw_version"] = "1.0.0";

    // Component configuration
    doc["name"] = "Minka Aire Fan";
    doc["unique_id"] = String(DEVICE_NAME) + "_fan";
    doc["command_topic"] = HA_FAN_COMMAND_TOPIC;
    doc["state_topic"] = HA_FAN_STATE_TOPIC;
    doc["availability_topic"] = HA_FAN_AVAILABILITY_TOPIC;
    doc["availability"] = true;

    // Speed control
    doc["speed_state_topic"] = HA_FAN_STATE_TOPIC;
    doc["speed_command_topic"] = HA_FAN_COMMAND_TOPIC + "/speed";
    doc["percentage"] = true;
    doc["percentage_command_topic"] = HA_FAN_COMMAND_TOPIC + "/percentage";

    // Preserve state
    doc["retain"] = true;
    doc["payload_on"] = "on";
    doc["payload_off"] = "off";

    // Serialize and publish
    DynamicJsonDocument buffer(1024);
    serializeJson(doc, buffer);

    client.publish(HA_FAN_CONFIG_TOPIC, buffer.c_str(), true);
    Serial.println("Published fan discovery payload");
}

// ============================================================================
// LIGHT DISCOVERY
// ============================================================================

void HADiscovery::publishLightConfig(PubSubClient& client) {
    StaticJsonDocument<512> doc;

    // Device information
    JsonObject device = doc.createNestedObject("device");
    device["name"] = "Minka Aire Fan";
    device["identifiers"] = DEVICE_NAME;
    device["manufacturer"] = "Minka Aire";
    device["model"] = "433MHz Remote Control";
    device["sw_version"] = "1.0.0";

    // Light configuration
    doc["name"] = "Minka Aire Light";
    doc["unique_id"] = String(DEVICE_NAME) + "_light";
    doc["command_topic"] = HA_LIGHT_COMMAND_TOPIC;
    doc["state_topic"] = HA_LIGHT_STATE_TOPIC;
    doc["availability_topic"] = HA_LIGHT_AVAILABILITY_TOPIC;
    doc["availability"] = true;

    // Preserve state
    doc["retain"] = true;
    doc["payload_on"] = "on";
    doc["payload_off"] = "off";

    // Serialize and publish
    DynamicJsonDocument buffer(1024);
    serializeJson(doc, buffer);

    client.publish(HA_LIGHT_CONFIG_TOPIC, buffer.c_str(), true);
    Serial.println("Published light discovery payload");
}

// ============================================================================
// AVAILABILITY
// ============================================================================

void HADiscovery::publishAvailability(PubSubClient& client, const char* status) {
    client.publish(HA_FAN_AVAILABILITY_TOPIC, status, true);
    client.publish(HA_LIGHT_AVAILABILITY_TOPIC, status, true);
    Serial.print("Published availability status: ");
    Serial.println(status);
}