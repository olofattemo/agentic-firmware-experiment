#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <stdio.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

struct sensor_data {
	struct sensor_value temp;
	struct sensor_value hum;
	int64_t last_update_time;
};

static struct sensor_data shared_sensor_data;
K_MUTEX_DEFINE(sensor_mutex);

void monitor_thread_entry(void *p1, void *p2, void *p3) {
	bool error_flag = false;

	if (!gpio_is_ready_dt(&led)) {
		printf("LED device not ready\n");
		return;
	}
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

	while (1) {
		k_sleep(K_MSEC(1000));

		int64_t current_time = k_uptime_get();
		k_mutex_lock(&sensor_mutex, K_FOREVER);
		int64_t last_time = shared_sensor_data.last_update_time;
		k_mutex_unlock(&sensor_mutex);

		if ((current_time - last_time) > 30000) {
			if (!error_flag) {
				error_flag = true;
				gpio_pin_set_dt(&led, 1);
				printf("Error: Sensor update timeout (flag set)\n");
			}
		} else {
			if (error_flag) {
				error_flag = false;
				gpio_pin_set_dt(&led, 0);
				printf("Info: Sensor updates resumed (flag cleared)\n");
			}
		}
	}
}

K_THREAD_DEFINE(monitor_tid, 1024, monitor_thread_entry, NULL, NULL, NULL, 7, 0, 0);

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

	k_mutex_lock(&sensor_mutex, K_FOREVER);
	shared_sensor_data.last_update_time = k_uptime_get();
	k_mutex_unlock(&sensor_mutex);

	while (1) {
		struct sensor_value temp, hum;
		bool success = true;

		if (sensor_sample_fetch(dev) < 0) {
			printf("Failed to fetch sample\n");
			success = false;
		} else {
			if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
				printf("Failed to get temperature\n");
				success = false;
			} else {
				printf("Temperature: %.2f C\n", sensor_value_to_double(&temp));
			}

			if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
				printf("Failed to get humidity\n");
				success = false;
			} else {
				printf("Humidity: %.2f %%\n", sensor_value_to_double(&hum));
			}
		}

		if (success) {
			k_mutex_lock(&sensor_mutex, K_FOREVER);
			shared_sensor_data.temp = temp;
			shared_sensor_data.hum = hum;
			shared_sensor_data.last_update_time = k_uptime_get();
			k_mutex_unlock(&sensor_mutex);
		}

		k_sleep(K_SECONDS(10));
	}

	return 0;
}
