#include "motor.h"
#include "timer.h"
#include <avr/io.h>
#include <util/delay.h>

static uint32_t motor_hz[MOTOR_COUNT] = {MOTOR_DEFAULT_HZ, MOTOR_DEFAULT_HZ};

bool motor_init(void)
{
    /* 电机1：STEP D11/PB5/OC1A；DIR D12/PB6。 */
    PORTB &= ~((1 << PORTB5) | (1 << PORTB6));
    DDRB |= (1 << DDB5) | (1 << DDB6);
    /* 电机2：STEP D6/PH3/OC4A；DIR D7/PH4。 */
    PORTH &= ~((1 << PORTH3) | (1 << PORTH4));
    DDRH |= (1 << DDH3) | (1 << DDH4);

    bool ok1 = step_timer_init(MOTOR_1, motor_hz[MOTOR_1]);
    bool ok2 = step_timer_init(MOTOR_2, motor_hz[MOTOR_2]);
    return ok1 && ok2;
}

bool motor_is_running(motor_id_t id)
{
    return (unsigned)id < MOTOR_COUNT && step_timer_is_running((uint8_t)id);
}

void motor_start(motor_id_t id)
{
    if((unsigned)id < MOTOR_COUNT){
        step_timer_start((uint8_t)id);
    }
}

void motor_stop(motor_id_t id)
{
    if((unsigned)id < MOTOR_COUNT){
        step_timer_stop((uint8_t)id);
    }
}

bool motor_set_speed(motor_id_t id, uint32_t hz)
{
    if((unsigned)id >= MOTOR_COUNT || hz < MOTOR_MIN_HZ || hz > MOTOR_MAX_HZ){
        return false;
    }
    bool running = motor_is_running(id);
    motor_stop(id);
    bool ok = step_timer_set_hz((uint8_t)id, hz);
    if(ok){
        motor_hz[id] = hz;
    }
    if(running){
        motor_start(id);
    }
    return ok;
}

void motor_change_direction(motor_id_t id)
{
    if((unsigned)id >= MOTOR_COUNT){
        return;
    }
    bool running = motor_is_running(id);
    motor_stop(id);
    /* 保留项目21的停脉冲换向方式，在 DIR 两侧均留出稳定时间。 */
    _delay_us(10);
    if(id == MOTOR_1){
        PORTB ^= (1 << PORTB6);
    } else {
        PORTH ^= (1 << PORTH4);
    }
    _delay_us(10);
    if(running){
        motor_start(id);
    }
}

uint32_t motor_get_speed(motor_id_t id)
{
    return (unsigned)id < MOTOR_COUNT ? motor_hz[id] : 0;
}

uint8_t motor_get_direction(motor_id_t id)
{
    if(id == MOTOR_1){
        return (PORTB & (1 << PORTB6)) != 0;
    }
    if(id == MOTOR_2){
        return (PORTH & (1 << PORTH4)) != 0;
    }
    return 0;
}
