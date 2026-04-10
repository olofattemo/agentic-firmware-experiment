#ifndef CONSOLE_INTERFACE_H
#define CONSOLE_INTERFACE_H

#include <stdint.h>

/**
 * @brief Broadcast normal weather update to console.
 * Implements: LLR-WX-022, LLR-WX-023
 */
void console_broadcast_weather(uint32_t uptime, float temp, float hum);

/**
 * @brief Broadcast communication loss warning to console.
 * Implements: LLR-WX-024, LLR-WX-025
 */
void console_broadcast_error(uint32_t uptime);

#endif // CONSOLE_INTERFACE_H
