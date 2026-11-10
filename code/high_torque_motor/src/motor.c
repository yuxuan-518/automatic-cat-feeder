/* Adapted from code/22_stepper_acceleration_ramp in the reference repository.
 * State machine runs in main; only the pulse generator runs in an ISR.
 */
#include "motor.h"
#include "motor_io.h"

static motor_state_t state;
static uint32_t current_hz, target_hz, acceleration, remainder, last_update;
static bool reverse_pending;

static void follow_target(void)
{
    state = current_hz < target_hz ? MOTOR_ACCEL :
            current_hz > target_hz ? MOTOR_DECEL : MOTOR_CRUISE;
}

static void launch(void)
{
    current_hz = MOTOR_START_HZ;
    remainder = 0;
    motor_io_start(current_hz);
    follow_target();
}

void motor_init(void)
{
    motor_io_init();
    state = MOTOR_STOPPED;
    current_hz = MOTOR_START_HZ;
    target_hz = MOTOR_DEFAULT_TARGET_HZ;
    acceleration = MOTOR_ACCEL_DEFAULT;
    remainder = last_update = 0;
    reverse_pending = false;
}

void motor_update(uint32_t now)
{
    uint32_t elapsed = now - last_update;
    if(elapsed < MOTOR_UPDATE_MS){ return; }
    last_update = now;
    if(state == MOTOR_STOPPED){ return; }
    /* Serial help/status can block at 9600 baud. Never catch up in one jump. */
    if(elapsed > 50UL){ elapsed = 50UL; }

    uint32_t goal = (state == MOTOR_STOPPING || state == MOTOR_REVERSING)
                    ? MOTOR_START_HZ : target_hz;
    remainder += acceleration * elapsed;
    uint32_t delta = remainder / 1000UL;
    remainder %= 1000UL;
    if(current_hz < goal){
        current_hz += (goal - current_hz < delta) ? goal - current_hz : delta;
    } else if(current_hz > goal){
        current_hz -= (current_hz - goal < delta) ? current_hz - goal : delta;
    }
    motor_io_set_hz(current_hz);
    if(current_hz != goal){ return; }

    if(state == MOTOR_STOPPING || state == MOTOR_REVERSING){
        bool restart = state == MOTOR_REVERSING;
        motor_io_stop();
        if(reverse_pending){ motor_io_flip_direction(); }
        reverse_pending = false;
        remainder = 0;
        state = MOTOR_STOPPED;
        if(restart){ launch(); }
    } else {
        state = MOTOR_CRUISE;
    }
}

void motor_start(void)
{
    if(state == MOTOR_STOPPED){ launch(); }
    else if(state == MOTOR_STOPPING){
        if(reverse_pending){ state = MOTOR_REVERSING; }
        else { follow_target(); }
    }
}

void motor_stop(void)
{
    /* Explicit stop also cancels a queued reversal. */
    reverse_pending = false;
    if(state != MOTOR_STOPPED){ state = MOTOR_STOPPING; }
}

void motor_estop(void)
{
    motor_io_stop();
    state = MOTOR_STOPPED;
    current_hz = MOTOR_START_HZ;
    remainder = 0;
    reverse_pending = false;
}

void motor_reverse(void)
{
    if(state == MOTOR_STOPPED){ motor_io_flip_direction(); }
    else if(state == MOTOR_STOPPING){ reverse_pending = !reverse_pending; }
    else if(state == MOTOR_REVERSING){
        reverse_pending = false;
        follow_target();
    } else {
        reverse_pending = true;
        state = MOTOR_REVERSING;
    }
}

bool motor_set_target(uint32_t hz)
{
    if(hz < MOTOR_MIN_HZ || hz > MOTOR_MAX_HZ){ return false; }
    target_hz = hz;
    if(state == MOTOR_ACCEL || state == MOTOR_DECEL || state == MOTOR_CRUISE){
        follow_target();
    }
    return true;
}

bool motor_set_accel(uint32_t hz_per_s)
{
    /* No direct-speed mode on this initial large-motor test project. */
    if(hz_per_s == 0 || hz_per_s > MOTOR_ACCEL_MAX){ return false; }
    acceleration = hz_per_s;
    remainder = 0;
    return true;
}

motor_state_t motor_get_state(void){ return state; }
uint32_t motor_get_current_hz(void){ return state == MOTOR_STOPPED ? 0 : current_hz; }
uint32_t motor_get_target_hz(void){ return target_hz; }
uint32_t motor_get_accel(void){ return acceleration; }
uint8_t motor_get_direction(void){ return motor_io_direction(); }
