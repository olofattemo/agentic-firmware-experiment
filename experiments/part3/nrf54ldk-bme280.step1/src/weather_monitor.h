#ifndef WEATHER_MONITOR_H
#define WEATHER_MONITOR_H

/**
 * @brief Thread entry point for the staleness monitor component.
 * Implements: §4.2 Task Mapping (Staleness_Monitor_Task)
 */
void Weather_Monitor_Task_Entry(void *p1, void *p2, void *p3);

#endif // WEATHER_MONITOR_H
