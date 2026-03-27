---
document_id: DOC-SRS-001
title: Weather Sensor (BME280) — Software Requirements Specification
version: 1.0.0
---

# 1. Introduction

## 1.1 Scope

This document specifies the complete software requirements for the Weather Sensor monitoring subsystem based on the Bosch BME280 temperature and humidity sensor. It covers functional behavior, fault detection and degradation, software structure, interface definitions, and configuration parameters. It serves as the single authoritative specification from which implementation, unit tests, and integration tests are derived. The hardware boundary shall be defined by the combined DeviceTree (dts) hierarchy: The board devicetree and the project overlay, and these will serve as the simplified HSI for the project. Since the HSI is simplified this implies that no new HAL/Drivers should be developed as part of the project.
No safety goals are allocated to this subsystem.

# 2. Functional Requirements

## 2.1 Data Acquisition

| ID | Requirement | Verification | Trace |
| :--- | :--- | :--- | :--- |
| `SRS-WX-001` | The software shall initiate a combined temperature and humidity measurement from the sensor every 10s (± 100ms) using the RTOS sensor API. | Test: Instrument the measurement task; measure call timestamps over 10 consecutive cycles. Confirm period within 10s ± 100ms. | `SyRS-WX-001` |
| `SRS-WX-002` | The software shall retrieve compensated temperature and humidity values by invoking the RTOS sensor API functions `sensor_sample_fetch()` and `sensor_channel_get()`, relying on the in-tree driver for hardware-specific compensation. | Test: Inject known raw values into a mocked sensor driver; compare application-level output (tolerance: ± 0.1°C, ± 1% RH). | `SyRS-WX-001` |

## 2.2 Data Storage

| ID | Requirement | Constraints | Verification | Trace |
| :--- | :--- | :--- | :--- | :--- |
| `SRS-WX-003` | The software shall write the compensated temperature value to shared memory in degrees Celsius. | Resolution: 0.1°C. Data type per §5.1. | Test: Read shared memory after a measurement cycle; verify the value matches the expected compensated output. | `SyRS-WX-002` |
| `SRS-WX-004` | The software shall write the compensated relative humidity value to shared memory in percent relative humidity (RH). | Resolution: 1%. Data type per §5.1. | Test: Read shared memory after a measurement cycle; verify the value matches the expected compensated output. | `SyRS-WX-002` |
| `SRS-WX-005` | The software shall increment a monotonic update counter in shared memory atomically upon each successful write of temperature and humidity data. | Consumers use this field to detect freshness independently of the staleness monitor. | Test: Read shared memory twice across a measurement cycle boundary; verify the counter increments exactly once per successful update. | `SyRS-WX-002` |

## 2.3 Display Output

| ID | Requirement | Verification | Trace |
| :--- | :--- | :--- | :--- |
| `SRS-WX-006` | The software shall print the compensated temperature and humidity values to the standard serial console upon every successful measurement using standard RTOS print mechanisms (Exact format enforced in §5.2). | Test: Monitor console output during normal operation; verify format matches expected values. | `SyRS-WX-001` |
| `SRS-WX-007` | If the weather sensor error flag is active, the software shall suppress the normal data output and instead print an error notification to the console every 10 seconds (Exact format enforced in §5.2). | Test: Trigger staleness fault; verify normal output stops and error message appears every 10s. | `SyRS-WX-003` |

## 2.4 LED Error Indication

| ID | Requirement | Verification | Trace |
| :--- | :--- | :--- | :--- |
| `SRS-WX-008` | When the weather sensor error flag is active (see `SRS-WX-020`), the software shall assert the logical active state of the fault LED. | Test: Trigger a staleness fault; observe LED illumination within one monitor cycle. | `SyRS-WX-004` |
| `SRS-WX-009` | When the weather sensor error flag is cleared (see `SRS-WX-021`), the software shall deassert the logical active state of the fault LED. | Test: Recover from a staleness fault; observe LED extinguishes within one monitor cycle after error clears. | `SyRS-WX-004` |

# 3. Fault Detection and Degradation Requirements

## 3.1 Staleness Monitoring

| ID | Requirement | Constraints | Verification | Trace |
| :--- | :--- | :--- | :--- | :--- |
| `SRS-WX-020` | The software shall set the weather sensor error flag if the temperature and humidity values in shared memory have not been successfully updated for `STALENESS_TIMEOUT` elapsed time. | Default timeout: 30s (see `CAL-WX-005`). Timer starts from the timestamp of the last successful write to shared memory. | Test: Block the sensor driver from completing a read. Verify error flag asserts after 30s ± 1s. | `SyRS-WX-003` |
| `SRS-WX-021` | The software shall clear the weather sensor error flag when a successful sensor read and shared memory update occurs after the error has been set. | Immediate clear on first successful update — no recovery debounce. | Test: While error flag is active, restore normal sensor communication. Verify error flag clears on the next successful 10s measurement cycle. | `SyRS-WX-003` |

## 3.2 Communication Fault Handling

| ID | Requirement | Constraints | Verification | Trace |
| :--- | :--- | :--- | :--- | :--- |
| `SRS-WX-022` | If the sensor API returns an error code during the fetch operation, the software shall discard the reading, retain previous valid values, and allow the staleness timer to continue. | Do not update shared memory. | Test: Mock the sensor driver to return `-EIO`; verify shared memory retains prior values and counter does not advance. | `SyRS-WX-001`, `SyRS-WX-003` |
| `SRS-WX-023` | If the sensor API returns compensated data outside the valid physical range (T: −40 to +85°C, RH: 0 to 100%), the software shall discard the reading, retain previous valid values, and allow the staleness timer to continue. | Do not update shared memory. | Test: Mock the sensor driver to return out-of-range values; verify shared memory retains prior values and counter does not advance. | `SyRS-WX-001`, `SyRS-WX-002` |

## 3.3 Operational Modes

The weather sensor software operates in three modes:

**INIT.** Entered at power-on. The RTOS boot sequence automatically initializes the sensor driver via Devicetree bindings. The application tasks verify device readiness (e.g., `device_is_ready()`). Shared memory contains invalid sentinel values (temperature: `NaN` or `−999.0f`, humidity: `0xFF` or `−1`, counter: `0`). LED state is off. The system transitions to NORMAL upon the first successful measurement and shared memory write.

**NORMAL.** The software performs a measurement every 10s via the sensor API, writes to shared memory, and prints to the console. The monitor task evaluates staleness. LED is off. If the staleness timeout expires (no successful update for 30s), transitions to ERROR.

**ERROR.** The weather sensor error flag is active. LED is on, and standard console output is replaced by error messages. The software continues to attempt measurement every 10s. Upon the first successful measurement and shared memory write, transitions back to NORMAL and turns off the LED.

# 4. Software Structure

*This section describes the internal partitioning of the weather sensor software into components. It replaces a standalone Software Architecture Description for this subsystem.*

## 4.1 Component Partitioning

| Component | Layer | Responsibility | HSI Binding |
| :--- | :--- | :--- | :--- |
| `Weather_Sensor` | Application | Implements the reading and storing of temperature and humidity values via the Zephyr Sensor API. Outputs data to the console. | Bound to the Devicetree sensor node. |
| `Weather_Sensor_Monitor` | Application | Implements the staleness monitoring logic. Evaluates shared memory timestamps to set or clear the error flag, and controls LED indication. | Bound to the Devicetree LED node. |

## 4.2 Task Mapping

| Runnable | Component | Trigger | Priority |
| :--- | :--- | :--- | :--- |
| `Weather_Sensor_Task` | `Weather_Sensor` | OS periodic timer, 10s | Low |
| `Staleness_Monitor_Task` | `Weather_Sensor_Monitor` | OS periodic timer, 1s | Low |

# 5. Software Interfaces

*This section defines the C-language contracts between components. It replaces a standalone Interface Control Document for this subsystem. Each interface corresponds to a dependency in §4.1.*

## 5.1 Shared Memory Interface (Weather Data Output)

```c
// File: Weather_SharedMem.h
// Owner: Weather_Sensor (writer)
// Consumers: Display task, logging task, telemetry task (readers)

#include <stdint.h>
#include <stdbool.h>

typedef float float32_t;

/**
 * Shared memory structure for weather sensor data.
 *
 * Atomicity contract: The writer (Weather_Sensor) guarantees that
 * temperature, humidity, and update_counter are updated atomically.
 * Implementation shall use a mutex, double-buffer, or sequence counter
 * to prevent consumers from observing partial updates (Atomicity contract).
 */
typedef struct {
    float32_t temperature_degC;   // Compensated temperature in °C.
                                  //   Valid range: [-40.0, +85.0] in NORMAL.
                                  //   Sentinel: -999.0f in INIT (no data yet).
    uint8_t   humidity_pctRH;     // Compensated relative humidity in % RH.
                                  //   Valid range: [0, 100] in NORMAL.
                                  //   Sentinel: 0xFF in INIT.
    uint32_t  update_counter;     // Monotonically incrementing counter.
                                  //   Advances by 1 on each successful write.
                                  //   Value 0 = no update has occurred (INIT).
    bool      error_active;       // true if staleness timeout has expired.
} Weather_SharedData_t;

/**
 * @brief  Get a pointer to the shared weather data structure.
 *
 * @return Pointer to the singleton shared data instance.
 * Lifetime: static, valid for the entire application runtime.
 *
 * @note   Readers must check update_counter or use the provided
 * mutex/sequence-counter mechanism before consuming data.
 */
const volatile Weather_SharedData_t* Weather_GetSharedData(void);
```

## 5.2 UART Interface (Display Output)

```c
// Interface Mechanism: Zephyr standard console output
// Owner: Zephyr OS (provider)
// Consumer: Weather_Sensor (caller)

/**
 * The application SHALL exclusively use the Zephyr `printk()` 
 * subsystem (`<zephyr/sys/printk.h>`) for all console output. 
 * Direct UART access or use of the Zephyr Logging API is prohibited.
 *
 * ------------------------------------------------------------------
 * BUILD DEPENDENCIES (prj.conf)
 * ------------------------------------------------------------------
 * The implementation requires the following Kconfig symbol to support 
 * floating-point conversion in printk:
 * - CONFIG_CBPRINTF_FP_SUPPORT=y  
 *
 * ------------------------------------------------------------------
 * EXACT FORMAT SPECIFICATIONS
 * ------------------------------------------------------------------
 * Normal Measurement Output (SRS-WX-006):
 * Format String: "T: %.1f C, RH: %u %%\n"
 * Example Output: "T: 22.4 C, RH: 45 %\n"
 * Note: Must include the trailing newline.
 *
 * Error Output (SRS-WX-007):
 * Format String: "Error: Sensor communication failure\n"
 * Note: Must include the trailing newline.
 */
```

# 6. Configuration Parameters

All parameters below shall be stored in a dedicated `.rodata` or configuration section. They are compile-time constants in the initial release.

| Parameter ID | Symbol | Default | Range | Used By | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `CAL-WX-001` | `MEASUREMENT_PERIOD_S` | 10 | 1 – 3600 | `SRS-WX-001` | Measurement cycle period in seconds |
| `CAL-WX-002` | `TEMP_RESOLUTION_DEGC` | 0.1 | — | `SRS-WX-003` | Temperature storage resolution (°C) |
| `CAL-WX-003` | `HUMID_RESOLUTION_PCT` | 1 | — | `SRS-WX-004` | Humidity storage resolution (% RH) |
| `CAL-WX-004` | `MONITOR_CHECK_PERIOD_S` | 1 | 1 – 10 | `SRS-WX-020` | Staleness monitor evaluation period in seconds |
| `CAL-WX-005` | `STALENESS_TIMEOUT_S` | 30 | 1 – 3600 | `SRS-WX-020` | Time without update before error flag is set |
