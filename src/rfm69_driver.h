/**
 * RFM69 Driver for 433MHz FeatherWing
 *
 * Wrapper around Adafruit RFM69 library for ESP32-C6 Feather
 * Handles initialization and transmission of 433MHz RF signals
 */

#ifndef RFM69_DRIVER_H
#define RFM69_DRIVER_H

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

// Note: The Adafruit RFM69 library is loaded via PlatformIO lib_deps
// The library header will be available as <RFM69.h>

class RFM69Driver {
public:
    /**
     * Initialize the RFM69HCW radio module
     * @return true if initialization successful, false otherwise
     */
    bool begin();

    /**
     * Transmit a fan speed signal
     * @param speed Speed level (1-3)
     */
    void transmitSignal(int speed);

    /**
     * Transmit fan off signal
     */
    void transmitFanOff();

    /**
     * Transmit light on signal
     */
    void transmitLightOn();

    /**
     * Transmit light off signal
     */
    void transmitLightOff();

private:
    bool initialized = false;

    /**
     * Configure RFM69 for 433MHz operation
     */
    void configureRF();

    /**
     * Transmit raw signal data (internal use)
     * @return true if transmission successful
     */
    bool transmitRaw(const uint8_t* data, uint8_t length);

    /**
     * Send a signal with retry logic
     * @param data Pointer to signal data array
     * @param length Length of the signal data
     * @param retries Number of transmission retries
     */
    void sendWithRetry(const uint8_t* data, uint8_t length, uint8_t retries = 3);
};

#endif // RFM69_DRIVER_H