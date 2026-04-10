#include "console_interface.h"
#include <zephyr/sys/printk.h>
#include <stdio.h>

/**
 * Implements: LLR-WX-022, LLR-WX-023
 */
void console_broadcast_weather(uint32_t uptime, float temp, float hum)
{
    // Implements: LLR-WX-023 - format [%u] T: %.2f C, H: %.2f %%
    // Note: Zephyr's printk supports float if CONFIG_CBPRINTF_FP_SUPPORT is set
    printk("[%u] T: %.2f C, H: %.2f %%\n", uptime, (double)temp, (double)hum);
}

/**
 * Implements: LLR-WX-024, LLR-WX-025
 */
void console_broadcast_error(uint32_t uptime)
{
    // Implements: LLR-WX-025 - format [%u] ERROR: Sensor communication lost
    printk("[%u] ERROR: Sensor communication lost\n", uptime);
}
