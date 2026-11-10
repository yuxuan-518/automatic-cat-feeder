#include <avr/interrupt.h>
#include <util/atomic.h>
#include "usart.h"
#include "timer.h"
#include "motor.h"
#include "command/command.h"
#include "command/command_motor.h"

/* 0：沿用项目21，串口输入 start 后启动；1：上电即启动两台。 */
#define START_BOTH_ON_BOOT 0

volatile uint32_t system_ms = 0;

ISR(TIMER3_COMPA_vect)
{
    system_ms++;
}

int main(void)
{
    usart0_init();
    bool ready = motor_init();
    if(!timer3_init(1000)){
        ready = false;
    }
    sei();

    if(!ready){
        motor_stop(MOTOR_1);
        motor_stop(MOTOR_2);
        usart0_send_string("ERROR timer init, motors stopped\r\n");
        while(1){}
    }

#if START_BOTH_ON_BOOT
    motor_start(MOTOR_1);
    motor_start(MOTOR_2);
    usart0_send_string("BOOT, both motors started\r\n");
#else
    usart0_send_string("BOOT, both motors stopped; type start or help\r\n");
#endif

    char command[COMMAND_LEN] = "";
    uint32_t last_serial = 0;
    while(1){
        uint32_t now;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
            now = system_ms;
        }
        if(command_receive(command)){
            command_motor_execute(command);
        }
        if(now - last_serial >= 2000UL){
            last_serial = now;
            command_motor_send_status();
        }
    }
}
