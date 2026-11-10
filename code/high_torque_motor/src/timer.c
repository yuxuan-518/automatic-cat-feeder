#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

static volatile uint32_t system_ms;
ISR(TIMER3_COMPA_vect){ system_ms++; }

void timer3_init(void)
{
    TCCR3A = 0;
    TCCR3B = _BV(WGM32);
    TCNT3 = 0;
    OCR3A = (uint16_t)(F_CPU / 64UL / 1000UL - 1UL);
    TIFR3 = _BV(OCF3A);
    TIMSK3 = _BV(OCIE3A);
    TCCR3B |= _BV(CS31) | _BV(CS30);
}

uint32_t timer_now_ms(void)
{
    uint32_t now;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){ now = system_ms; }
    return now;
}
