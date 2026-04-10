#include "weather_monitor.h"
#include "Weather_SharedMem.h"
#include "console_interface.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(weather_monitor);

// Implements: SRS-WX-011 (Constraint: use led0 alias from devicetree)
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/**
 * Implements: LLR-WX-015, LLR-WX-016, LLR-WX-017, LLR-WX-018, 
 *             LLR-WX-019, LLR-WX-020, LLR-WX-021
 */
void weather_monitor_task_entry(void *p1, void *p2, void *p3)
{
    int ret;
    bool in_error_state = false; // ASSUMPTION: maintain state to detect transitions

    // Implements: LLR-WX-015 - configure led0 as output
    if (!gpio_is_ready_dt(&led)) {
        LOG_ERR("LED device not ready");
        return;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure LED: %d", ret);
        return;
    }

    while (1) {
        struct weather_data_t data;
        uint32_t now = k_uptime_get_32();

        // Implements: LLR-WX-017 - read most recent data record
        weather_shared_mem_read(&data);

        // Implements: LLR-WX-018 - calculate staleness
        uint32_t staleness = now - data.timestamp;

        // Implements: LLR-WX-019, LLR-WX-020
        if (data.valid && (staleness < 30000)) {
            // Normal state
            // Implements: LLR-WX-019 - command LED to inactive
            gpio_pin_set_dt(&led, 0);
            in_error_state = false;
        } else {
            // Communication Loss state
            // Implements: LLR-WX-020 - command LED to active
            gpio_pin_set_dt(&led, 1);

            // Implements: LLR-WX-021 - invoke error output once on transition
            if (!in_error_state) {
                console_broadcast_error(now);
                in_error_state = true;
            }
        }

        // Implements: LLR-WX-016 - pause for exactly 1,000 ms
        k_msleep(1000);
    }
}
