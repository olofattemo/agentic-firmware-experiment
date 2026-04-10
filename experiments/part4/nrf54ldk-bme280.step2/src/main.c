#include <zephyr/kernel.h>
#include "weather_sensor.h"
#include "weather_monitor.h"

// Implements: SAD-3.2 Tasking and Priorities
#define STACK_SIZE 1024
#define PRIORITY 7

K_THREAD_DEFINE(weather_sensor_tid, STACK_SIZE,
                weather_sensor_task_entry, NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(weather_monitor_tid, STACK_SIZE,
                weather_monitor_task_entry, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int main(void)
{
    // Tasks started by K_THREAD_DEFINE
    return 0;
}
