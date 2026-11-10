#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

/* id 0: Timer1 / OC1A / D11; id 1: Timer4 / OC4A / D6. */
bool step_timer_init(uint8_t id, uint32_t hz);
bool step_timer_set_hz(uint8_t id, uint32_t hz);
void step_timer_start(uint8_t id);
void step_timer_stop(uint8_t id);
bool step_timer_is_running(uint8_t id);
bool timer3_init(uint32_t hz);

#endif
