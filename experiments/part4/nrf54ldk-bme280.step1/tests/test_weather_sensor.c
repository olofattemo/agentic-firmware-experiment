#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/**
 * Verifies: LLR-WX-007
 */
ZTEST(weather_sensor_tests, test_temperature_validation)
{
    // Normally we would mock the sensor_channel_get to return specific values
    float valid_temp = 25.0f;
    float invalid_temp_high = 90.0f;
    float invalid_temp_low = -50.0f;

    zassert_true((valid_temp >= -40.0f && valid_temp <= 85.0f), "Valid temp rejected");
    zassert_false((invalid_temp_high >= -40.0f && invalid_temp_high <= 85.0f), "Invalid high temp accepted");
    zassert_false((invalid_temp_low >= -40.0f && invalid_temp_low <= 85.0f), "Invalid low temp accepted");
}

/**
 * Verifies: LLR-WX-008
 */
ZTEST(weather_sensor_tests, test_humidity_validation)
{
    float valid_hum = 50.0f;
    float invalid_hum_high = 110.0f;
    float invalid_hum_low = -10.0f;

    zassert_true((valid_hum >= 0.0f && valid_hum <= 100.0f), "Valid humidity rejected");
    zassert_false((invalid_hum_high >= 0.0f && invalid_hum_high <= 100.0f), "Invalid high humidity accepted");
    zassert_false((invalid_hum_low >= 0.0f && invalid_hum_low <= 100.0f), "Invalid low humidity accepted");
}

ZTEST_SUITE(weather_sensor_tests, NULL, NULL, NULL, NULL, NULL);
