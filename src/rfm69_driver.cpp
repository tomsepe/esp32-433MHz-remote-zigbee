/**
 * RFM69 Driver Implementation for 433MHz FeatherWing
 *
 * Wraps Adafruit RFM69 library for ESP32-C6 Feather
 */

#include "rfm69_driver.h"
#include "RFM69.h"  // Adafruit RFM69 library

// RFM69 instance
RFM69 radio = RFM69();

// ============================================================================
// INITIALIZATION
// ============================================================================

bool RFM69Driver::begin() {
    Serial.println("Initializing SPI bus...");

    // Initialize SPI with custom pins for ESP32-C6
    SPI.begin(RFM69_SPI_SCK, RFM69_SPI_MISO, RFM69_SPI_MOSI, RFM69_CS);

    Serial.println("Initializing RFM69 radio...");

    // Initialize RFM69 at 433MHz
    // Parameters: frequency, nodeID, networkID
    if (!radio.initialize(RFM69_FREQUENCY, RFM69_NODEID, RFM69_NETWORKID)) {
        Serial.println("ERROR: RFM69 initialization failed!");
        return false;
    }

    // Configure RF settings
    configureRF();

    // Enable promiscuous mode for raw transmission (bypasses encryption/ACK)
    radio.promiscuous(true);

    initialized = true;
    Serial.println("RFM69 initialized successfully at 433MHz");

    return true;
}

// ============================================================================
// RF CONFIGURATION
// ============================================================================

void RFM69Driver::configureRF() {
    // Set transmission power (max 22dBm for RFM69HCW)
    radio.setTxPower(RFM69_TX_POWER, false);

    // Configure for 433MHz operation
    // The RFM69HCW supports 315MHz, 433MHz, 868MHz, 915MHz
    Serial.print("Configured for ");
    Serial.print(RFM69_FREQUENCY);
    Serial.println(" MHz");
}

// ============================================================================
// TRANSMISSION FUNCTIONS
// ============================================================================

void RFM69Driver::transmitSignal(int speed) {
    if (!initialized) {
        Serial.println("ERROR: RFM69 not initialized");
        return;
    }

    // Select signal based on speed
    const uint8_t* signal = nullptr;
    uint8_t length = 0;

    switch (speed) {
        case 1:
            signal = SIGNAL_FAN_SPEED_1;
            length = SIGNAL_FAN_SPEED_1_LEN;
            break;
        case 2:
            signal = SIGNAL_FAN_SPEED_2;
            length = SIGNAL_FAN_SPEED_2_LEN;
            break;
        case 3:
            signal = SIGNAL_FAN_SPEED_3;
            length = SIGNAL_FAN_SPEED_3_LEN;
            break;
        default:
            Serial.print("ERROR: Invalid speed ");
            Serial.println(speed);
            return;
    }

    // Transmit with retry for reliability
    sendWithRetry(signal, length, 3);
}

void RFM69Driver::transmitFanOff() {
    if (!initialized) {
        Serial.println("ERROR: RFM69 not initialized");
        return;
    }
    sendWithRetry(SIGNAL_FAN_OFF, SIGNAL_FAN_OFF_LEN, 3);
}

void RFM69Driver::transmitLightOn() {
    if (!initialized) {
        Serial.println("ERROR: RFM69 not initialized");
        return;
    }
    sendWithRetry(SIGNAL_LIGHT_ON, SIGNAL_LIGHT_ON_LEN, 3);
}

void RFM69Driver::transmitLightOff() {
    if (!initialized) {
        Serial.println("ERROR: RFM69 not initialized");
        return;
    }
    sendWithRetry(SIGNAL_LIGHT_OFF, SIGNAL_LIGHT_OFF_LEN, 3);
}

// ============================================================================
// RAW TRANSMISSION
// ============================================================================

bool RFM69Driver::transmitRaw(const uint8_t* data, uint8_t length) {
    if (!initialized) {
        Serial.println("ERROR: RFM69 not initialized");
        return false;
    }

    if (data == nullptr || length == 0) {
        Serial.println("ERROR: Invalid data");
        return false;
    }

    // Send raw data via RFM69
    // The RFM69 send function transmits the payload
    radio.send(0xFF, data, length);  // 0xFF = broadcast to all nodes

    // Wait for transmission to complete
    delay(10);

    return true;
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void RFM69Driver::sendWithRetry(const uint8_t* data, uint8_t length, uint8_t retries) {
    Serial.print("Transmitting signal (");
    Serial.print(retries);
    Serial.println(" retries)");

    for (uint8_t i = 0; i < retries; i++) {
        Serial.print("  Attempt ");
        Serial.println(i + 1);

        (void)transmitRaw(data, length);

        // Wait between transmissions to match remote timing
        delay(100);
    }

    Serial.println("Transmission complete");
}