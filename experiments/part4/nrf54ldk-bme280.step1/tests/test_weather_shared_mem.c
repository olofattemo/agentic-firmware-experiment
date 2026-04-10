#include <zephyr/ztest.h>
#include "../src/Weather_SharedMem.h"

/**
 * Verifies: LLR-WX-010, LLR-WX-012, LLR-WX-013, LLR-WX-014
 */
ZTEST(weather_shared_mem_tests, test_write_read_cycle)
{
    struct weather_data_t read_back;
    float test_temp = 25.5f;
    float test_hum = 50.0f;
    uint32_t test_ts = 12345;

    // Act
    weather_shared_mem_write(test_temp, test_hum, test_ts);
    weather_shared_mem_read(&read_back);

    // Assert
    zassert_equal(read_back.temperature, test_temp, "Temperature mismatch");
    zassert_equal(read_back.humidity, test_hum, "Humidity mismatch");
    zassert_equal(read_back.timestamp, test_ts, "Timestamp mismatch");
    zassert_true(read_back.valid, "Validity flag not set");
}

ZTEST_SUITE(weather_shared_mem_tests, NULL, NULL, NULL, NULL, NULL);
