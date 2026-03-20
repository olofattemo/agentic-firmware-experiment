#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>

int main(void)
{
	const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme280);

	if (dev == NULL) {
		printf("No BME280 device found\n");
		return 0;
	}

	if (!device_is_ready(dev)) {
		printf("Device %s is not ready\n", dev->name);
		return 0;
	}

	printf("Found device %s, starting measurements...\n", dev->name);

	while (1) {
		struct sensor_value temp, hum;

		if (sensor_sample_fetch(dev) < 0) {
			printf("Failed to fetch sample\n");
			k_sleep(K_SECONDS(10));
			continue;
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
			printf("Failed to get temperature\n");
		} else {
			printf("Temperature: %.2f C\n", sensor_value_to_double(&temp));
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
			printf("Failed to get humidity\n");
		} else {
			printf("Humidity: %.2f %%\n", sensor_value_to_double(&hum));
		}

		k_sleep(K_SECONDS(10));
	}

	return 0;
}