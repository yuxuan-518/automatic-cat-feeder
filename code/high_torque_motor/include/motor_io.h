#ifndef MOTOR_IO_H
#define MOTOR_IO_H
#include <stdint.h>
void motor_io_init(void);
void motor_io_start(uint32_t hz);
void motor_io_set_hz(uint32_t hz);
void motor_io_stop(void);
void motor_io_flip_direction(void);
uint8_t motor_io_direction(void);
#endif
