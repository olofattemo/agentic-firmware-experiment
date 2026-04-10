# Traceability Matrix

| LLR ID | Parent SRS | Implementing File/Function | Verifying Test |
|--------|------------|----------------------------|----------------|
| LLR-WX-001 | SRS-WX-001 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Sensor initialization over I2C). |
| LLR-WX-002 | SRS-WX-001 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (RTOS scheduling and `k_msleep` behavior). |
| LLR-WX-003 | SRS-WX-001 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Hardware sensor fetch API). |
| LLR-WX-004 | SRS-WX-001 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Hardware sensor channel retrieval). |
| LLR-WX-005 | SRS-WX-002 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; inline conversion is tightly coupled to hardware sensor fetch loop. |
| LLR-WX-006 | SRS-WX-003 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; inline conversion is tightly coupled to hardware sensor fetch loop. |
| LLR-WX-007 | SRS-WX-007 | `weather_sensor.c` / `weather_sensor_task_entry` | `tests/test_weather_sensor.c` |
| LLR-WX-008 | SRS-WX-008 | `weather_sensor.c` / `weather_sensor_task_entry` | `tests/test_weather_sensor.c` |
| LLR-WX-009 | SRS-WX-001, 007, 008 | `weather_sensor.c` / `weather_sensor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Integration across thread boundary). |
| LLR-WX-010 | SRS-WX-005 | `Weather_SharedMem.c` / static struct | `tests/test_weather_shared_mem.c` |
| LLR-WX-011 | SRS-WX-006 | `Weather_SharedMem.c` / `weather_mutex` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Requires RTOS concurrent execution environment). |
| LLR-WX-012 | SRS-WX-005 | `Weather_SharedMem.c` / `weather_shared_mem_write` | `tests/test_weather_shared_mem.c` |
| LLR-WX-013 | SRS-WX-005, 006 | `Weather_SharedMem.c` / `weather_shared_mem_write` | `tests/test_weather_shared_mem.c` |
| LLR-WX-014 | SRS-WX-005, 006 | `Weather_SharedMem.c` / `weather_shared_mem_read` | `tests/test_weather_shared_mem.c` |
| LLR-WX-015 | SRS-WX-011 | `weather_monitor.c` / `weather_monitor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (GPIO configuration). |
| LLR-WX-016 | SRS-WX-009 | `weather_monitor.c` / `weather_monitor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (RTOS scheduling and `k_msleep` behavior). |
| LLR-WX-017 | SRS-WX-009 | `weather_monitor.c` / `weather_monitor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Integration with shared memory in thread). |
| LLR-WX-018 | SRS-WX-009 | `weather_monitor.c` / `weather_monitor_task_entry` | `tests/test_weather_monitor.c` |
| LLR-WX-019 | SRS-WX-012 | `weather_monitor.c` / `weather_monitor_task_entry` | `tests/test_weather_monitor.c` |
| LLR-WX-020 | SRS-WX-009, 011 | `weather_monitor.c` / `weather_monitor_task_entry` | `tests/test_weather_monitor.c` |
| LLR-WX-021 | SRS-WX-010 | `weather_monitor.c` / `weather_monitor_task_entry` | BLOCKED: Cannot be tested as unit test; must be tested on actual hardware (Integration crossing to console thread). |
| LLR-WX-022 | SRS-WX-004 | `console_interface.c` / `console_broadcast_weather` | `tests/test_console_interface.c` |
| LLR-WX-023 | SRS-WX-002, 003, 004 | `console_interface.c` / `console_broadcast_weather` | `tests/test_console_interface.c` |
| LLR-WX-024 | SRS-WX-010 | `console_interface.c` / `console_broadcast_error` | `tests/test_console_interface.c` |
| LLR-WX-025 | SRS-WX-010 | `console_interface.c` / `console_broadcast_error` | `tests/test_console_interface.c` |
