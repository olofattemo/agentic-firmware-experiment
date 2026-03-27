#include "weather_monitor.h"
#include "Weather_SharedMem.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Implements: CAL-WX-004
#define MONITOR_CHECK_PERIOD_S 1

// Implements: CAL-WX-005
#define STALENESS_TIMEOUT_S 30

// Implements: SRS-WX-008, SRS-WX-009 (LED node)
#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void Weather_Monitor_Task_Entry(void *p1, void *p2, void *p3) {
    // Implements: §3.3 Operational Modes (INIT)
    if (!gpio_is_ready_dt(&led)) {
        return;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    while (1) {
        const volatile Weather_SharedData_t* shared = Weather_GetSharedData();
        int64_t now = k_uptime_get();
        
        // Implements: SRS-WX-020 (Staleness Timeout Check)
        if ((now - shared->last_update_ms) > K_SECONDS(STALENESS_TIMEOUT_S).ticks) {
            // Implements: SRS-WX-020 (Set error flag)
            Weather_SetErrorActive(true);

            // Implements: SRS-WX-008 (Assert fault LED)
            gpio_pin_set_dt(&led, 1);
        } else {
            // Implements: SRS-WX-021 (Clear error flag)
            Weather_SetErrorActive(false);

            // Implements: SRS-WX-009 (Deassert fault LED)
            gpio_pin_set_dt(&led, 0);
        }

        // Implements: CAL-WX-004 (Evaluation period 1s)
        k_sleep(K_SECONDS(MONITOR_CHECK_PERIOD_S));
    }
}
