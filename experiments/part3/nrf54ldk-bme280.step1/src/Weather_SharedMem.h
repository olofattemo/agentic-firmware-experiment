#ifndef WEATHER_SHARED_MEM_H
#define WEATHER_SHARED_MEM_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

// Implements: SRS-WX-003, SRS-WX-004
typedef float float32_t;

/**
 * Shared memory structure for weather sensor data.
 * Implements: §5.1 Shared Memory Interface
 */
typedef struct {
    float32_t temperature_degC;   // Compensated temperature in °C.
    uint8_t   humidity_pctRH;     // Compensated relative humidity in % RH.
    uint32_t  update_counter;     // Monotonically incrementing counter.
    bool      error_active;       // true if staleness timeout has expired.
    int64_t   last_update_ms;     // Internal: Timestamp of last successful update
} Weather_SharedData_t;

/**
 * @brief  Get a pointer to the shared weather data structure.
 * Implements: §5.1 Shared Memory Interface
 *
 * @return Pointer to the singleton shared data instance.
 */
const volatile Weather_SharedData_t* Weather_GetSharedData(void);

/**
 * @brief Update the shared weather data.
 * Internal to Weather_Sensor component.
 * Implements: §5.1 Atomicity contract
 */
void Weather_UpdateSharedData(float32_t temp, uint8_t humidity);

/**
 * @brief Set the error flag in shared weather data.
 * Internal to Staleness_Monitor component.
 * Implements: §5.1 Atomicity contract
 */
void Weather_SetErrorActive(bool active);

#endif // WEATHER_SHARED_MEM_H
