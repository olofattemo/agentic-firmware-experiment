#include "weather_sensor.h"
#include "Weather_SharedMem.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

// Implements: CAL-WX-001
#define MEASUREMENT_PERIOD_S 10

// Implements: SRS-WX-023 Physical Range Constants
#define TEMP_MIN_DEGC -40.0f
#define TEMP_MAX_DEGC 85.0f
#define HUMID_MIN_PCT 0
#define HUMID_MAX_PCT 100

void Weather_Sensor_Task_Entry(void *p1, void *p2, void *p3) {
    const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme280);

    // Implements: §3.3 Operational Modes (INIT)
    if (dev == NULL || !device_is_ready(dev)) {
        printk("Error: Device \"%s\" is not ready; check the devicetree.\n", dev->name);
        return;
    }

    while (1) {
        struct sensor_value temp, humid;
        int rc;

        // Implements: SRS-WX-001 (Measurement Period 10s)
        int64_t next_run = k_uptime_get() + K_SECONDS(MEASUREMENT_PERIOD_S).ticks;

        // Implements: SRS-WX-002 (Retrieve data via sensor_sample_fetch)
        rc = sensor_sample_fetch(dev);

        if (rc == 0) {
            // Implements: SRS-WX-002 (sensor_channel_get)
            sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
            sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humid);

            float32_t t_val = sensor_value_to_double(&temp);
            float32_t h_val = sensor_value_to_double(&humid);

            // Implements: SRS-WX-023 (Physical Range Check)
            if (t_val >= TEMP_MIN_DEGC && t_val <= TEMP_MAX_DEGC &&
                h_val >= (float32_t)HUMID_MIN_PCT && h_val <= (float32_t)HUMID_MAX_PCT) {
                
                // Implements: SRS-WX-003, SRS-WX-004, SRS-WX-005 (Update shared memory)
                Weather_UpdateSharedData(t_val, (uint8_t)h_val);
            } else {
                // Implements: SRS-WX-023 (Discard invalid readings)
            }
        } else {
            // Implements: SRS-WX-022 (Discard reading on sensor fetch error)
        }

        const volatile Weather_SharedData_t* shared = Weather_GetSharedData();

        // Implements: SRS-WX-006, SRS-WX-007, §5.2 (UART output format)
        if (shared->error_active) {
            // Implements: SRS-WX-007 (Error format)
            printk("Error: Sensor communication failure\n");
        } else if (shared->update_counter > 0) {
            // Implements: SRS-WX-006 (Normal format)
            printk("T: %.1f C, RH: %u %%\n", 
                   (double)shared->temperature_degC, 
                   (unsigned int)shared->humidity_pctRH);
        }

        // Wait for next cycle
        k_sleep(K_TIMEOUT_ABS_MS(next_run));
    }
}
