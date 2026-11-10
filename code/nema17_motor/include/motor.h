#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>
#include <stdbool.h>

#define MOTOR_COUNT 2
#define MOTOR_DEFAULT_HZ 100UL
#define MOTOR_MIN_HZ 50UL
#define MOTOR_MAX_HZ 8000UL

/* API 编号从 0 开始；串口命令编号从 1 开始。 */
typedef enum { MOTOR_1 = 0, MOTOR_2 = 1 } motor_id_t;

bool motor_init(void);
void motor_start(motor_id_t id);
void motor_stop(motor_id_t id);
bool motor_set_speed(motor_id_t id, uint32_t hz);
void motor_change_direction(motor_id_t id);
bool motor_is_running(motor_id_t id);
uint32_t motor_get_speed(motor_id_t id);
uint8_t motor_get_direction(motor_id_t id);

#endif
