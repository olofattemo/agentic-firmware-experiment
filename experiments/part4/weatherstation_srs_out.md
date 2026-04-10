# Weather Station SRS

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

## 5. Open Questions and Assumptions
- **OPEN QUESTION**: Should the system continue to output the last known good value to the console during communication loss, or stop reporting until recovery? (Current SRS assumes reporting stops and is replaced by an error message).
- **ASSUMPTION**: The "serial console" refers to the default Zephyr console configured in the project (RTT).
- **ASSUMPTION**: The hardware indicator (LED) refers to the devicetree node aliased as `led0` on the nRF54L15-DK.
- **ASSUMPTION**: "Physically valid" ranges are defined by the sensor's operational limits (-40 to 85°C for temperature, 0-100% for humidity).
