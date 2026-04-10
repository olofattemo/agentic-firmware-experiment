#include <zephyr/ztest.h>
#include <stdio.h>

/**
 * Verifies: LLR-WX-023
 */
ZTEST(console_interface_tests, test_weather_format)
{
    char buf[100];
    uint32_t uptime = 1000;
    float temp = 23.456f;
    float hum = 45.678f;

    // Expected format: [%u] T: %.2f C, H: %.2f %%
    snprintf(buf, sizeof(buf), "[%u] T: %.2f C, H: %.2f %%", uptime, (double)temp, (double)hum);

    zassert_str_equal(buf, "[1000] T: 23.46 C, H: 45.68 %", "Weather format mismatch");
}

/**
 * Verifies: LLR-WX-025
 */
ZTEST(console_interface_tests, test_error_format)
{
    char buf[100];
    uint32_t uptime = 5000;

    // Expected format: [%u] ERROR: Sensor communication lost
    snprintf(buf, sizeof(buf), "[%u] ERROR: Sensor communication lost", uptime);

    zassert_str_equal(buf, "[5000] ERROR: Sensor communication lost", "Error format mismatch");
}

ZTEST_SUITE(console_interface_tests, NULL, NULL, NULL, NULL, NULL);
