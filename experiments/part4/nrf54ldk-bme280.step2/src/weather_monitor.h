#ifndef WEATHER_MONITOR_H
#define WEATHER_MONITOR_H

/**
 * @brief Thread entry point for the staleness monitor task.
 * Implements: LLR-WX-016
 */
void weather_monitor_task_entry(void *p1, void *p2, void *p3);

#endif // WEATHER_MONITOR_H
