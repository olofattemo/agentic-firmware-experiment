#ifndef WEATHER_SHARED_MEM_H
#define WEATHER_SHARED_MEM_H

#include <zephyr/kernel.h>
#include <stdbool.h>
#include <stdint.h>

// Implements: ICD-4.1
struct weather_data_t {
    float temperature; // Celsius
    float humidity;    // Percent RH
    uint32_t timestamp; // Kernel uptime in ms
    bool valid;
};

/**
 * @brief Write weather data to shared memory.
 * Implements: LLR-WX-012
 */
void weather_shared_mem_write(float temp, float hum, uint32_t timestamp);

/**
 * @brief Read weather data from shared memory.
 * Implements: LLR-WX-014
 */
void weather_shared_mem_read(struct weather_data_t *dest);

#endif // WEATHER_SHARED_MEM_H
