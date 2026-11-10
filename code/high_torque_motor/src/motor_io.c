#include "motor_io.h"
#include "motor.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

/* Mega D23=PA1 -> PU+; D22=PA0 -> DR+. PU-/DR- -> Mega GND.
 * D23 has no OC1 output, so Timer1 interrupts toggle PA1 in software.
 * Normal mode avoids lowering CTC TOP below TCNT1 during a speed change.
 * 16 MHz / 8 = 2 MHz; one interrupt per half-period, two per pulse.
 */
#define STEP_BIT PA1
#define DIR_BIT PA0
static volatile uint16_t half_period_ticks;

ISR(TIMER1_COMPA_vect)
{
    PORTA ^= _BV(STEP_BIT);
    OCR1A += half_period_ticks;
}

void motor_io_set_hz(uint32_t hz)
{
    /* The public motor API validates this; guard the hardware boundary too. */
    if(hz < MOTOR_MIN_HZ || hz > MOTOR_MAX_HZ){ return; }
    uint16_t ticks = (uint16_t)(F_CPU / (16UL * hz));
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){ half_period_ticks = ticks; }
}

void motor_io_init(void)
{
    TCCR1A = 0;
    TCCR1B = 0;
    TIMSK1 = 0;
    PORTA &= (uint8_t)~(_BV(STEP_BIT) | _BV(DIR_BIT));
    DDRA |= _BV(STEP_BIT) | _BV(DIR_BIT);
    motor_io_set_hz(MOTOR_START_HZ);
}

void motor_io_start(uint32_t hz)
{
    motor_io_set_hz(hz);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        PORTA &= (uint8_t)~_BV(STEP_BIT);
        TCNT1 = 0;
        OCR1A = half_period_ticks;
        TIFR1 = _BV(OCF1A);
        TIMSK1 = _BV(OCIE1A);
        TCCR1B = _BV(CS11);
    }
}

void motor_io_stop(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        TIMSK1 = 0;
        TCCR1B = 0;
        /* Finish any HIGH pulse before forcing idle LOW (driver min 2.5 us). */
        if(PORTA & _BV(STEP_BIT)){ _delay_us(10); }
        PORTA &= (uint8_t)~_BV(STEP_BIT);
        TIFR1 = _BV(OCF1A);
    }
}

void motor_io_flip_direction(void)
{
    /* Caller stops pulses first. Allow both DIR hold and setup time. */
    _delay_us(20);
    PORTA ^= _BV(DIR_BIT);
    _delay_us(20);
}

uint8_t motor_io_direction(void)
{
    return (PORTA & _BV(DIR_BIT)) != 0;
}
