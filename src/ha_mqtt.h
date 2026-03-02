/**
 * Home Assistant MQTT Integration
 *
 * MQTT topic definitions and Home Assistant MQTT Discovery support
 */

#ifndef HA_MQTT_H
#define HA_MQTT_H

#include <PubSubClient.h>
#include <ArduinoJson.h>

// ============================================================================
// MQTT TOPIC DEFINITIONS
// ============================================================================

// Base topic for all Home Assistant communication
#define HA_MQTT_BASE "homeassistant"

// Fan component topics
#define HA_FAN_CONFIG_TOPIC HA_MQTT_BASE "/fan/minka_fan/config"
#define HA_FAN_COMMAND_TOPIC HA_MQTT_BASE "/fan/minka_fan/command"
#define HA_FAN_STATE_TOPIC HA_MQTT_BASE "/fan/minka_fan/state"
#define HA_FAN_AVAILABILITY_TOPIC HA_MQTT_BASE "/fan/minka_fan/availability"
#define HA_FAN_PERCENTAGE_TOPIC HA_FAN_COMMAND_TOPIC "/percentage"

// Light component topics
#define HA_LIGHT_CONFIG_TOPIC HA_MQTT_BASE "/light/minka_light/config"
#define HA_LIGHT_COMMAND_TOPIC HA_MQTT_BASE "/light/minka_light/command"
#define HA_LIGHT_STATE_TOPIC HA_MQTT_BASE "/light/minka_light/state"
#define HA_LIGHT_AVAILABILITY_TOPIC HA_MQTT_BASE "/light/minka_light/availability"

// ============================================================================
// SHORT TOPIC ALIASES (for internal use)
// ============================================================================

#define MQTT_FAN_TOPIC HA_FAN_COMMAND_TOPIC
#define MQTT_LIGHT_TOPIC HA_LIGHT_COMMAND_TOPIC
#define MQTT_AVAILABILITY_TOPIC HA_FAN_AVAILABILITY_TOPIC

// ============================================================================
// MQTT DISCOVERY STRUCTURES
// ============================================================================

/**
 * Publish Home Assistant MQTT Discovery payload
 * This allows automatic device discovery in Home Assistant
 */
class HADiscovery {
public:
    /**
     * Publish fan discovery payload
     * @param client MQTT client reference
     */
    static void publishFanConfig(PubSubClient& client);

    /**
     * Publish light discovery payload
     * @param client MQTT client reference
     */
    static void publishLightConfig(PubSubClient& client);

    /**
     * Publish availability status
     * @param client MQTT client reference
     * @param status "online" or "offline"
     */
    static void publishAvailability(PubSubClient& client, const char* status);

    /**
     * Publish fan state (on/off, speed/percentage)
     * @param client MQTT client reference
     * @param state "on" or "off"
     * @param percentage 0-100 (0=off, 1-33=low, 34-66=med, 67-100=high)
     */
    static void publishFanState(PubSubClient& client, const char* state, int percentage);

    /**
     * Publish light state (on/off)
     * @param client MQTT client reference
     * @param state "on" or "off"
     */
    static void publishLightState(PubSubClient& client, const char* state);
};

// ============================================================================
// MQTT CALLBACK DEFINITION
// ============================================================================

// Forward declaration of MQTT callback (implemented in main.cpp)
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif // HA_MQTT_H