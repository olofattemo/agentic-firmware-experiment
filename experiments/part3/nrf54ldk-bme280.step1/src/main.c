#include <zephyr/kernel.h>
#include "Weather_SharedMem.h"
#include "weather_sensor.h"
#include "weather_monitor.h"

// Implements: §4.2 Task Mapping
#define STACK_SIZE 1024
#define PRIORITY 7

K_THREAD_DEFINE(weather_sensor_tid, STACK_SIZE,
                Weather_Sensor_Task_Entry, NULL, NULL, NULL,
                PRIORITY, 0, 0);

K_THREAD_DEFINE(staleness_monitor_tid, STACK_SIZE,
                Weather_Monitor_Task_Entry, NULL, NULL, NULL,
                PRIORITY, 0, 0);

int main(void)
{
    // Tasks started by K_THREAD_DEFINE
    return 0;
}
