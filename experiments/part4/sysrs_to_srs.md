You are a systems engineer producing a Software Requirements Specification 
for an embedded firmware project targeting the nRF54L15-DK board running 
Zephyr RTOS.

## Inputs

System Requirements (authoritative - do not modify):
|ID|System Requirement|
|------------|------------|
|`SyRS-WX-001`|The system shall acquire ambient temperature and humidity measurements every 10 seconds and output the readings to the serial console.|
|`SyRS-WX-002`|The system shall make the most recent, physically valid temperature and humidity measurements continuously available to other internal system functions.|
|`SyRS-WX-003`|The system shall detect a sensor communication loss if valid data is not received for 30 seconds, and notify the user via the serial console.|
|`SyRS-WX-004`|The system shall provide a continuous visual warning via a dedicated hardware indicator (LED) while the sensor communication loss condition is active.|

Hardware-Software Interface:
The HSI is the project devicetree at nrf54l15dk_nrf54l15_cpuapp.overlay
combined with the base board DTS. Treat the devicetree as the authoritative
source for pin assignments, peripheral instances, and bus configuration. Do
not propose alternative pin allocations or peripheral mappings.

## Output

Produce a single Markdown document titled "Weather Station SRS" at 
specs/weatherstation_srs.md with the 
following sections. Use IEEE 29148 terminology and structure conventions 
for the SRS section, and IEEE 1016 conventions for the SAD section. Tailor 
the format for a small embedded application - produce only the sections 
needed for the inputs given, not a complete compliance deliverable.

1. Scope and References
   - Brief description of what this software does
   - References to the SyRS and the devicetree overlay

2. Software Requirements (SRS)
   - Derive software-level requirements from each SyRS item
   - Use IDs of the form SRS-WX-NNN (start at 001, increment by 1)
   - Each requirement must be a single testable "shall" statement
   - Cover functional behavior, error handling, data validity ranges, 
     output formatting, and timing
   - For each SRS requirement, note in parentheses which SyRS it derives 
     from, e.g. "(derives from SyRS-WX-001)"

3. Software Architectural Description (SAD)
   - Decompose the software into named components
   - For each component, state its responsibility in one sentence
   - Identify which SRS requirements each component is responsible for
   - Specify task structure (threads, periods, priorities) where relevant
   - Constrain the implementation to use existing Zephyr drivers and 
     subsystems — do not introduce new HAL or driver code

4. Interface Control Document (ICD)
   - Define internal interfaces between components (function signatures, 
     shared data structures, synchronization primitives)
   - Define external interfaces (console output format with exact field 
     names, separators, and units; LED behavior)
   - Be specific about format strings and field widths so that downstream 
     code generation cannot drift on cosmetic details

## Constraints

- Do not invent hardware details not present in the devicetree
- Do not reference pin numbers or peripheral instances directly; refer to 
  devicetree node labels or aliases
- Do not propose driver implementations
- Mark any assumption you make explicitly with "ASSUMPTION:" so the human 
  reviewer can find it
- If a SyRS requirement is ambiguous, list the ambiguity at the top of the 
  document under "Open Questions" rather than silently making a choice
