#ifndef MOTOR_H
#define MOTOR_H
#include <stdbool.h>
#include <stdint.h>

/* CL86Y-V20: SW3=ON, SW4=OFF, SW5=ON, SW6=ON. */
#define MOTOR_PULSES_PER_REV 1600UL
#define MOTOR_START_HZ 50UL
#define MOTOR_MIN_HZ 50UL
#define MOTOR_MAX_HZ 2000UL
#define MOTOR_DEFAULT_TARGET_HZ 400UL
#define MOTOR_ACCEL_LOW 100UL
#define MOTOR_ACCEL_HIGH 500UL
#define MOTOR_ACCEL_DEFAULT 200UL
#define MOTOR_ACCEL_MAX 2000UL
#define MOTOR_UPDATE_MS 10UL

typedef enum {
    MOTOR_STOPPED, MOTOR_ACCEL, MOTOR_CRUISE, MOTOR_DECEL,
    MOTOR_STOPPING, MOTOR_REVERSING
} motor_state_t;

void motor_init(void);
void motor_update(uint32_t now);
void motor_start(void);
void motor_stop(void);
void motor_estop(void);
void motor_reverse(void);
bool motor_set_target(uint32_t hz);
bool motor_set_accel(uint32_t hz_per_s);
motor_state_t motor_get_state(void);
uint32_t motor_get_current_hz(void);
uint32_t motor_get_target_hz(void);
uint32_t motor_get_accel(void);
uint8_t motor_get_direction(void);
#endif
