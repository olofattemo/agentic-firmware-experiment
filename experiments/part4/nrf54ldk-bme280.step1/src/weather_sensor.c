#include "weather_sensor.h"
#include "Weather_SharedMem.h"
#include "console_interface.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(weather_sensor);

// Implements: SRS-WX-001 (Constraint: use BME280 node from devicetree)
#define BME280_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(bosch_bme280)

/**
 * Implements: LLR-WX-001, LLR-WX-002, LLR-WX-003, LLR-WX-004, 
 *             LLR-WX-005, LLR-WX-006, LLR-WX-007, LLR-WX-008, LLR-WX-009
 */
void weather_sensor_task_entry(void *p1, void *p2, void *p3)
{
    // Implements: LLR-WX-001 - Initialize sensor device
    const struct device *const dev = DEVICE_DT_GET(BME280_NODE);

    if (!device_is_ready(dev)) {
        LOG_ERR("BME280 device not ready");
        // Implements: LLR-WX-001 - Error state (thread stops)
        return;
    }

    while (1) {
        struct sensor_value temp_val, hum_val;
        int rc;

        // Implements: LLR-WX-003 - trigger a new reading
        rc = sensor_sample_fetch(dev);
        if (rc == 0) {
            // Implements: LLR-WX-004 - retrieve channel values
            sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val);
            sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum_val);

            // Implements: LLR-WX-005 - convert to float
            float temp = (float)sensor_value_to_double(&temp_val);
            // Implements: LLR-WX-006 - convert to float
            float hum = (float)sensor_value_to_double(&hum_val);

            // Implements: LLR-WX-007 - validate temperature range [-40, 85]
            bool temp_valid = (temp >= -40.0f) && (temp <= 85.0f);
            // Implements: LLR-WX-008 - validate humidity range [0, 100]
            bool hum_valid = (hum >= 0.0f) && (hum <= 100.0f);

            if (temp_valid && hum_valid) {
                // Implements: LLR-WX-009 - write to shared memory
                uint32_t now = k_uptime_get_32();
                weather_shared_mem_write(temp, hum, now);

                // Implements: SRS-WX-004 - output to console
                console_broadcast_weather(now, temp, hum);
            }
        } else {
            // Implements: LLR-WX-003 - bypass and wait for next cycle
            LOG_WRN("Failed to fetch sensor sample: %d", rc);
        }

        // Implements: LLR-WX-002 - pause for exactly 10,000 ms
        k_msleep(10000);
    }
}
