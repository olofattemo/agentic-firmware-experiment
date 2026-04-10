#include "Weather_SharedMem.h"

// Implements: LLR-WX-010
static struct weather_data_t weather_data = {
    .temperature = 0.0f,
    .humidity = 0.0f,
    .timestamp = 0,
    .valid = false
};

// Implements: LLR-WX-011
K_MUTEX_DEFINE(weather_mutex);

/**
 * Implements: LLR-WX-012, LLR-WX-013
 */
void weather_shared_mem_write(float temp, float hum, uint32_t timestamp)
{
    // Implements: LLR-WX-013 - block until mutex is acquired
    k_mutex_lock(&weather_mutex, K_FOREVER);

    // Implements: LLR-WX-013 - update all fields
    weather_data.temperature = temp;
    weather_data.humidity = hum;
    weather_data.timestamp = timestamp;
    weather_data.valid = true;

    // Implements: LLR-WX-013 - release mutex
    k_mutex_unlock(&weather_mutex);
}

/**
 * Implements: LLR-WX-014
 */
void weather_shared_mem_read(struct weather_data_t *dest)
{
    // Implements: LLR-WX-014 - block until mutex is acquired
    k_mutex_lock(&weather_mutex, K_FOREVER);

    // Implements: LLR-WX-014 - copy contents
    if (dest != NULL) {
        dest->temperature = weather_data.temperature;
        dest->humidity = weather_data.humidity;
        dest->timestamp = weather_data.timestamp;
        dest->valid = weather_data.valid;
    }

    // Implements: LLR-WX-014 - release mutex
    k_mutex_unlock(&weather_mutex);
}
