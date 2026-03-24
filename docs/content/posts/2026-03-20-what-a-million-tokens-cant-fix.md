---
title: "Too Much or Too Little: What a Million Tokens Can't Fix"
date: 2026-03-20T14:01:00Z
draft: false
tags: ["Agentic Workflows", "Firmware", "LLM", "ARM Cortex-M", "Context Management"]
summary: "Starting from a naive vibe coding attempt that exposes how LLMs confuse pin allocations for newer hardware, this post walks through iteratively building a working BME280 weather station on the nRF54L15-DK. We explore how context quality affects code generation and show that even correct results degrade as underspecified details drift between iterations. The takeaway is that both prompt clarity and detail persistence matter as much as model capability."
---

## Naive Example: Vibe Coding a Weather Station

For our naive baseline example we will use Gemini CLI with Gemini 3.1 Pro as the model. First, we initialise a new application with the nRF Connect SDK on the nRF54L15-DK,
and connect a BME280 sensor board over I2C using P1.11 and P1.12 since these are unallocated in the base board. Then I asked Gemini for another free pin to use for chip select.

> **Prompt:** nRF54L15-DK free IO pins except P1.11 and P1.12

Gemini said:
> "Since you are using the nRF54L15-DK ... and you want to avoid P1.11 and P1.12 (which are typically mapped to LED 3 and LED 4 on this board), here is your 'Safe-to-Use' list..."*

Then follows a long answer with a list of "free" PN.M ports in Nordic numbering that looks like it could be correct but is easily verifiable to be completely wrong.

> **Prompt:** You are confusing the pin allocation on nRF54L15-DK with some other board? See allocation in table [...]*

Gemini said:
> "You're absolutely right to call me out on that. I was pulling from general Nordic DK layouts, and the nRF54L15-DK has a specific (and quite different) pin allocation ...*"

\* abbreviated

What is a "general Nordic DK layout"? you might wonder. Well, even though we are using a very capable LLM here, this shows that it is not trained on the pin allocation of the nRF54L15-DK, or that the phrasing or details provided in the question do not lead us to the correct output. So the model infers the answer using a different but semantically similar set of data.

Since the nRF54L15 is relatively new it could also be that stronger correlations with an older DK that occurs more frequently in the training data cause this behaviour. The result is an answer that is statistically plausible but factually wrong. This is similar to what I described in "LLMs trained on evolving interfaces" in the last post. Here the DK pinout is the interface, but referring to the new nRF54L15 version is not enough to produce a useful result.

## Improving Results with Context

To correct this kind of behaviour without retraining the model (which we cannot do in our setup) we need to add the correct information to the model context, i.e. give the model the correct information within the current session. 
How this can be done varies, but many harnesses (such as Claude Code or Gemini CLI) can load this from special markdown files within the current directory (agent configuration), or based on a specific mode of operation (skills), or they can automatically maintain and record memories.
We can also inject the context in the session using tools that fetch it from the filesystem or databases, or write the information as text in the current chat session.

At the time of writing, most frontline models support 1M tokens (a token can be roughly approximated to one word in a sentence) in their "context window". That is a lot of context in addition to the query or "prompt" that we provide when looking for an answer.
We could front-load the model with datasheets, code, specifications, and preferences to get our prompts to correlate strongly with the most relevant details. 

Maintaining or "curating" the context, however, becomes a challenge since we must know enough about the answers we want, to be able to gauge if our context additions improve the model. Front-loading sparse or conflicting information can actually make the results worse. The goal, in information theory terms, is to maximise the signal-to-noise ratio of the information relevant to solving our problems.

## Generating code for the Weather Station example

We create a new application for our board nRF54L15-DK and target config `nrf54l15/cpuapp/ns`. This configures sysbuild with TF-M boot and an empty application project directory with a `main.c` and the base DTS. We then create an overlay that allocates P1.11 and P1.12 for the BME280 I2C bus and P1.06 for CS, and wire them to a MIKROE-1978 Weather Click board. Then we disable the second UART to free the CS pin for our use and redirect the console output to the onboard RTT for simplicity. The [base template (step1)](https://github.com/olofattemo/agentic-firmware-experiment/tree/main/experiments/part2/nrf54ldk-bme280.step1) with an empty main.c, board base DTS and our application overlay should give an LLM that can navigate a Zephyr repo a reasonable context for code generation.

**Initial main.c:** 
```c
#include <zephyr/kernel.h>

int main(void)
{
        return 0;
}
```

We point Gemini CLI to the project directory and try the naive prompt:

> **Prompt:** Read the repository that contains an empty application for the nRF54L15 on the nRF54L15-DK board. Create an application that reads temperature and humidity from the attached BME280 via I2C every 10 s. The application shall correct the readings and print them to the serial console as degrees C and percent relative humidity respectively.

<details>
<summary>Full Gemini response:</summary>


> I have updated the project configuration and source code to meet your requirements. 
>
>  Here is what was done:
>   1. prj.conf: Enabled the required Zephyr subsystems and drivers (CONFIG_I2C, CONFIG_SENSOR, CONFIG_BME280) as well as floating-point
>      support for printf to accurately output the compensated values.
>
>   2. main.c: Added the required includes and implemented an infinite loop that runs every 10 seconds. It fetches new sample data from
>      the bosch,bme280 device, reads both the ambient temperature and humidity channels (which are automatically corrected using the
>      BME280's factory calibration registers by the driver), and prints the results to the serial console as degrees Celsius and percent
>      relative humidity, respectively.
</details>

\
**Updated main.c:** 
```c
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
```
Full output: [step2](https://github.com/olofattemo/agentic-firmware-experiment/tree/main/experiments/part2/nrf54ldk-bme280.step2)

Impressively enough, given a correct overlay and project configuration, this produces a working code example that prints the sensor data to the console.

**Console output:**
```
Temperature: 23.27 C
Humidity: 30.26 %
Temperature: 23.27 C
Humidity: 30.22 %
...
```

## Adding Monitoring

> **Prompt:** Store the temperature and humidity readings in a shared variable protected from concurrent access. Monitor the values and set an error flag if they have not been updated for 30 s. When the error flag is set, LED1 should be turned on. If the updates resume, the flag is cleared and LED1 is turned off again.

<details>
<summary>Full Gemini response:</summary>


 I have implemented the requested functionality by updating your application to safely share and monitor the sensor data. Here is a
  summary of the changes:

   1. Shared State & Mutex: I introduced a struct sensor_data to hold the temperature and humidity readings along with a timestamp of the
      last update. This shared variable is protected from concurrent access using a Zephyr mutex (K_MUTEX_DEFINE(sensor_mutex)).
   2. Monitor Thread: I added a dedicated background thread (monitor_thread_entry) that runs every second to check the timestamp of the
      last update.
   3. Error Flag & LED Control: 
       * If the difference between the current system uptime and the last_update_time exceeds 30 seconds, an error_flag is set and LED1
         (led0 in the devicetree) is turned on to visually indicate a timeout.
       * Once the updates resume (i.e., new sensor values are fetched successfully), the last_update_time is refreshed, the thread clears
         the error flag, and LED1 is turned off.
   4. Configuration Update: I enabled CONFIG_GPIO=y in your prj.conf to ensure the Zephyr GPIO APIs used for toggling the LED are
      included in the build.

  The system will now securely manage the measurements while providing a visible alert if the BME280 sensor stops responding for more
  than 30 seconds.
</details>

\
**Updated main.c:**

```c
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
```
Full output: [step3](https://github.com/olofattemo/agentic-firmware-experiment/tree/main/experiments/part2/nrf54ldk-bme280.step3)

This also produced a working example. The monitoring could be confirmed to work by physically detaching the SCL wire, ensuring the LED lit up, and plugging it back in again
to make sure it recovers.

![nRF54L15-DK with BME280 in step 3](https://olofattemo.github.io/agentic-firmware-experiment/images/nrf54l15dk-bme280-step3.jpg)

I was expecting LED1 to map to `led1` (since we do have `led0`–`led3` on the board). Otherwise this instruction was pretty well received.

## Unintended changes

Our example shows that the LLM is trained on the Zephyr codebase and examples, and it can reasonably correlate build configuration and decode the DTS where we provided a correct initial input, if given access to a repository. The code only uses hardware abstractions that protect us from the most difficult pitfalls.

However, as we iterate and add more requirements, they fill up the context. The LLM will eventually need to free up context space. It will likely do so either by dropping some of the early details or by summarising the entire context so far. By doing so, it will start forgetting about details like: what LED should we turn on when there is an error reading temperature?

There are also details that are underspecified, that were inferred by the LLM. For example: How many decimals should the temperature and humidity be printed with? Forgotten and inferred details are more likely to change when we ask for a modification that touches the same area at some later point in time. This is likely to be unacceptable in a real project.

> **Prompt:**  Change the console output so that temperature and humidity are both printed simultaneously on the same line

Gemini said:
> I have combined the console output into a single line. The program now fetches both channels first and then prints them together if
>  successful:
>   ```c
>   printf("Temp: %.2f C, Hum: %.2f %%\n",
>          sensor_value_to_double(&temp),
>          sensor_value_to_double(&hum));
>   ```
Full output: [step4](https://github.com/olofattemo/agentic-firmware-experiment/tree/main/experiments/part2/nrf54ldk-bme280.step4)

Imagine now that the console is our interface to the display. Instead of:
> Temperature: 23.27 C \
> Humidity: 30.26 %

The output at the other end is now:

> Temp: 23.27 C, Hum: 30.26 % 

This is exactly the kind of drift that underspecified requirements invite. Besides concatenating the previous two-line output. A comma was introduced as delimiter, and the labels were abbreviated. We did not explicitly ask for this. The model inferred it due to lack of clear instructions.

## Summary and Next Steps

The takeaway of this experiment is that besides model knowledge, context quality and clarity of instructions matter.

To bridge the gaps further, let's consider increasing the clarity of our prompts, or "requirements", as well as the persistence of the details. To some extent, it does not matter if the requirements are for human or LLM consumption. There is value in both persistence and clarity either way.

In real projects we often layer requirements in the stakeholder or user layer, system layer, and software layer. Then we further split them into functional and non-functional, design and interface requirements. How we do this and the exact language we use to describe the layers usually depends on what industry and what kind of certification we are targeting depending on the application.

Next time we will look at designing a software requirements specification structure and example to investigate if it helps to address our concerns about precision and persistence.
