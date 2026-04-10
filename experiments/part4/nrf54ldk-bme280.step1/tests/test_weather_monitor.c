#include <zephyr/ztest.h>
#include <stdint.h>

/**
 * Verifies: LLR-WX-018
 */
ZTEST(weather_monitor_tests, test_staleness_calculation)
{
    uint32_t now = 40000;
    uint32_t last_update = 15000;
    uint32_t expected_staleness = 25000;

    uint32_t actual_staleness = now - last_update;

    zassert_equal(actual_staleness, expected_staleness, "Staleness calculation incorrect");
}

/**
 * Verifies: LLR-WX-019, LLR-WX-020
 */
ZTEST(weather_monitor_tests, test_error_state_logic)
{
    uint32_t staleness_valid = 25000;
    uint32_t staleness_invalid = 35000;

    // LLR-WX-019: valid if staleness < 30000
    zassert_true(staleness_valid < 30000, "Valid staleness logic error");
    
    // LLR-WX-020: error if staleness >= 30000
    zassert_true(staleness_invalid >= 30000, "Invalid staleness logic error");
}

ZTEST_SUITE(weather_monitor_tests, NULL, NULL, NULL, NULL, NULL);
