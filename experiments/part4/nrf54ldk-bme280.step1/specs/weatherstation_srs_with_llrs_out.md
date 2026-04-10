# Weather Station SRS with LLRs

## 1. Scope and References
### 1.1 Scope
This document defines the software requirements and architecture for the Weather Station firmware targeting the nRF54L15-DK board. The software is responsible for periodic environmental sensing (temperature and humidity) using a Bosch BME280 sensor, data distribution to internal system functions, and health monitoring with user notification via serial console and hardware indicators.

### 1.2 References
- System Requirements (SyRS-WX-001 through SyRS-WX-004)
- Hardware Interface: `nrf54l15dk_nrf54l15_cpuapp_ns.overlay` and nRF54L15-DK board devicetree.

## 2. Software Requirements (SRS)
### 2.1 Functional Requirements
- **SRS-WX-001**: The software shall poll the BME280 sensor for temperature and humidity data at a period of 10.0 seconds. (derives from SyRS-WX-001)
- **SRS-WX-002**: The software shall format the acquired temperature as a floating-point value in degrees Celsius with 2 decimal places. (derives from SyRS-WX-001)
- **SRS-WX-003**: The software shall format the acquired humidity as a floating-point value in percent relative humidity with 2 decimal places. (derives from SyRS-WX-001)
- **SRS-WX-004**: The software shall output the formatted temperature and humidity to the serial console following every successful acquisition. (derives from SyRS-WX-001)
- **SRS-WX-005**: The software shall store the most recent temperature and humidity measurements in a shared memory structure accessible by other internal system functions. (derives from SyRS-WX-002)
- **SRS-WX-006**: The software shall ensure thread-safe access to shared weather data using Zephyr synchronization primitives (e.g., mutex). (derives from SyRS-WX-002)

### 2.2 Data Validity and Error Handling
- **SRS-WX-007**: The software shall consider temperature measurements valid only if they are within the range of -40.0°C to +85.0°C. (derives from SyRS-WX-002)
- **SRS-WX-008**: The software shall consider humidity measurements valid only if they are within the range of 0.0% to 100.0%. (derives from SyRS-WX-002)
- **SRS-WX-009**: The software shall enter a "Communication Loss" state if no valid sensor data has been received for 30 consecutive seconds. (derives from SyRS-WX-003)
- **SRS-WX-010**: The software shall output a "Sensor communication loss" warning message to the serial console upon entering the Communication Loss state. (derives from SyRS-WX-003)
- **SRS-WX-011**: The software shall activate the hardware indicator (LED) while the Communication Loss state is active. (derives from SyRS-WX-004)
- **SRS-WX-012**: The software shall deactivate the hardware indicator (LED) immediately upon receiving a valid sensor measurement while in the Communication Loss state. (derives from SyRS-WX-004)

## 3. Software Architectural Description (SAD)
### 3.1 Component Decomposition
- **Weather Sensor Component**: Manages the I2C interface to the BME280, handles periodic polling, and performs range validation. (Responsible for SRS-WX-001, 002, 003, 007, 008)
- **Weather Shared Memory Component**: Provides a thread-safe data structure for storing and retrieving the latest valid measurements and timestamps. (Responsible for SRS-WX-005, 006)
- **Weather Monitor Component**: Tracks data staleness, manages the Communication Loss state machine, and controls the warning indicator. (Responsible for SRS-WX-009, 011, 012)
- **Console Interface Component**: Standardizes the formatting of weather data and error messages for serial transmission. (Responsible for SRS-WX-004, 010)

### 3.2 Tasking and Priorities
- **Sensor Task**: A dedicated Zephyr thread with a priority of 7. It executes every 10 seconds to sample the BME280 via the Zephyr Sensor API.
- **Monitor Task**: A dedicated Zephyr thread with a priority of 7. It executes every 1 second to verify that the last successful update in Shared Memory is within the 30-second window.

### 3.3 Constraints
- Implementation shall use the standard Zephyr `sensor` subsystem for BME280 interaction.
- GPIO control for the indicator shall use the Zephyr `gpio` API targeting the `led0` devicetree alias.

## 4. Interface Control Document (ICD)
### 4.1 Internal Interfaces
- **Weather Data Structure**:
  ```c
  struct weather_data_t {
      float temperature; // Celsius
      float humidity;    // Percent RH
      uint32_t timestamp; // Kernel uptime in ms
      bool valid;
  };
  ```
- **Synchronization**: Access to the global instance of `weather_data_t` must be protected by a `k_mutex`.

### 4.2 External Interfaces
- **Serial Console Output**:
  - Regular update format: `[<uptime_ms>] T: <val> C, H: <val> %`
  - Error message format: `[<uptime_ms>] ERROR: Sensor communication lost`
- **Hardware Indicator (LED)**:
  - Active (Error): LED associated with alias `led0` is ON.
  - Inactive (Normal): LED associated with alias `led0` is OFF.

## 5. Low-Level Requirements (LLRs)

### 5.1 Weather Sensor Component
- **LLR-WX-001**: The Weather Sensor Component shall initialize the sensor device mapped to the BME280 node during system initialization, entering an error state if the device is not ready. (Derives from: SRS-WX-001)
- **LLR-WX-002**: The Weather Sensor Component shall execute a continuous task loop that pauses execution for exactly 10,000 milliseconds between measurement acquisitions. (Derives from: SRS-WX-001)
- **LLR-WX-003**: The Weather Sensor Component shall invoke the hardware sensor API to trigger a new reading; if the read operation fails, the component shall bypass value extraction and wait for the next periodic cycle. (Derives from: SRS-WX-001)
- **LLR-WX-004**: Upon successful data acquisition, the Weather Sensor Component shall retrieve the ambient temperature and humidity channel values from the sensor API. (Derives from: SRS-WX-001)
- **LLR-WX-005**: The Weather Sensor Component shall convert the retrieved temperature value into a floating-point data type representing degrees Celsius. (Derives from: SRS-WX-002)
- **LLR-WX-006**: The Weather Sensor Component shall convert the retrieved humidity value into a floating-point data type representing percent relative humidity. (Derives from: SRS-WX-003)
- **LLR-WX-007**: The Weather Sensor Component shall classify the acquired temperature as valid if and only if it satisfies the physical condition `temperature >= -40.0` AND `temperature <= 85.0`. (Derives from: SRS-WX-007)
- **LLR-WX-008**: The Weather Sensor Component shall classify the acquired humidity as valid if and only if it satisfies the physical condition `humidity >= 0.0` AND `humidity <= 100.0`. (Derives from: SRS-WX-008)
- **LLR-WX-009**: If both temperature and humidity are classified as valid, the Weather Sensor Component shall write the floating-point values and the current system uptime timestamp to the Weather Shared Memory Component. (Derives from: SRS-WX-001, SRS-WX-007, SRS-WX-008)

### 5.2 Weather Shared Memory Component
- **LLR-WX-010**: The Weather Shared Memory Component shall maintain a statically allocated internal memory structure of type `weather_data_t` to hold the most recent sensor state. (Derives from: SRS-WX-005)
- **LLR-WX-011**: The Weather Shared Memory Component shall maintain a mutual exclusion primitive (mutex) to protect the internal `weather_data_t` structure from concurrent access. (Derives from: SRS-WX-006)
- **LLR-WX-012**: The Weather Shared Memory Component shall expose a write interface that accepts a temperature value, a humidity value, and a timestamp. (Derives from: SRS-WX-005)
- **LLR-WX-013**: The write interface shall block until the mutex is acquired, update all fields of the internal memory structure, set its validity flag to true, and subsequently release the mutex. (Derives from: SRS-WX-005, SRS-WX-006)
- **LLR-WX-014**: The Weather Shared Memory Component shall expose a read interface that blocks until the mutex is acquired, copies the contents of the internal memory structure into a caller-provided destination structure, and subsequently releases the mutex. (Derives from: SRS-WX-005, SRS-WX-006)

### 5.3 Weather Monitor Component
- **LLR-WX-015**: The Weather Monitor Component shall configure the hardware indicator pin (aliased as `led0`) as a discrete digital output during system initialization. (Derives from: SRS-WX-011)
- **LLR-WX-016**: The Weather Monitor Component shall execute a continuous task loop that pauses execution for exactly 1,000 milliseconds between staleness evaluations. (Derives from: SRS-WX-009)
- **LLR-WX-017**: On each evaluation cycle, the Weather Monitor Component shall read the most recent data record via the Weather Shared Memory Component's read interface. (Derives from: SRS-WX-009)
- **LLR-WX-018**: The Weather Monitor Component shall calculate the data staleness by subtracting the record's timestamp from the current system uptime timestamp. (Derives from: SRS-WX-009)
- **LLR-WX-019**: If the retrieved data record is flagged as valid AND the calculated staleness is strictly less than 30,000 milliseconds, the Weather Monitor Component shall command the hardware indicator pin to its inactive state. (Derives from: SRS-WX-012)
- **LLR-WX-020**: If the calculated staleness is greater than or equal to 30,000 milliseconds, OR the retrieved data record is flagged as invalid, the Weather Monitor Component shall command the hardware indicator pin to its active state. (Derives from: SRS-WX-009, SRS-WX-011)
- **LLR-WX-021**: The Weather Monitor Component shall invoke the Console Interface Component's error output function exactly once when the system state transitions from a valid state to the communication loss state. (Derives from: SRS-WX-010)

### 5.4 Console Interface Component
- **LLR-WX-022**: The Console Interface Component shall expose an interface to broadcast the normal operational weather update, accepting the system uptime, temperature, and humidity as inputs. (Derives from: SRS-WX-004)
- **LLR-WX-023**: The normal operational weather update interface shall format and transmit the string `[%u] T: %.2f C, H: %.2f %%` to the standard output, utilizing the input arguments to populate the timestamp and measurement values to exactly two decimal places. (Derives from: SRS-WX-002, SRS-WX-003, SRS-WX-004)
- **LLR-WX-024**: The Console Interface Component shall expose an interface to broadcast the communication loss warning, accepting the current system uptime as input. (Derives from: SRS-WX-010)
- **LLR-WX-025**: The communication loss warning interface shall format and transmit the string `[%u] ERROR: Sensor communication lost` to the standard output, substituting `%u` with the input system uptime. (Derives from: SRS-WX-010)

## 6. Open Questions and Assumptions
- **OPEN QUESTION**: Should the system continue to output the last known good value to the console during communication loss, or stop reporting until recovery? (Current SRS assumes reporting stops and is replaced by an error message).
- **ASSUMPTION**: The "serial console" refers to the default Zephyr console configured in the project (RTT).
- **ASSUMPTION**: The hardware indicator (LED) refers to the devicetree node aliased as `led0` on the nRF54L15-DK.
- **ASSUMPTION**: "Physically valid" ranges are defined by the sensor's operational limits (-40 to 85°C for temperature, 0-100% for humidity).
- **ASSUMPTION**: The Weather Monitor Component maintains the previous evaluation state internally to detect transitions and ensure the error message is only printed upon entering the Communication Loss state, rather than spamming continuously.