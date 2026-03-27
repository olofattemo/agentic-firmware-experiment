#include "Weather_SharedMem.h"
#include <zephyr/kernel.h>
#include <zephyr/init.h>

// Implements: §5.1 Shared Memory Interface
static Weather_SharedData_t shared_data;
K_MUTEX_DEFINE(shared_mem_mutex);

/**
 * Implements: §3.3 Operational Modes (INIT state)
 */
static int Weather_SharedMem_Init(void) {
    k_mutex_lock(&shared_mem_mutex, K_FOREVER);
    shared_data.temperature_degC = -999.0f; // SRS-WX-003 sentinel
    shared_data.humidity_pctRH = 0xFF;      // SRS-WX-004 sentinel
    shared_data.update_counter = 0;         // SRS-WX-005 initialization
    shared_data.error_active = false;       // Default state
    shared_data.last_update_ms = -30000;    // Offset so it starts as "stale" until first update
    k_mutex_unlock(&shared_mem_mutex);
    return 0;
}

SYS_INIT(Weather_SharedMem_Init, APPLICATION, 0);

/**
 * Implements: §5.1 const volatile access
 */
const volatile Weather_SharedData_t* Weather_GetSharedData(void) {
    return &shared_data;
}

/**
 * Implements: SRS-WX-003, SRS-WX-004, SRS-WX-005, §5.1 Atomicity contract
 */
void Weather_UpdateSharedData(float32_t temp, uint8_t humidity) {
    k_mutex_lock(&shared_mem_mutex, K_FOREVER);
    shared_data.temperature_degC = temp;
    shared_data.humidity_pctRH = humidity;
    shared_data.update_counter++; // Implements: SRS-WX-005
    shared_data.last_update_ms = k_uptime_get(); // Update internal timestamp
    k_mutex_unlock(&shared_mem_mutex);
}

/**
 * Implements: §5.1 Atomicity contract, SRS-WX-020, SRS-WX-021
 */
void Weather_SetErrorActive(bool active) {
    k_mutex_lock(&shared_mem_mutex, K_FOREVER);
    shared_data.error_active = active;
    k_mutex_unlock(&shared_mem_mutex);
}
