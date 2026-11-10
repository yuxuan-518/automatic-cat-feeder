#include <avr/interrupt.h>
#include "motor.h"
#include "timer.h"
#include "usart.h"
#include "demo.h"
#include "command/command.h"
#include "command/command_motor.h"

int main(void)
{
    motor_init();
    timer3_init();
    usart0_init();
    sei();
    usart0_send_string("BOOT CL86Y-V20; STOPPED; STEP=D23 DIR=D22; type help\r\n");
    char line[COMMAND_LEN];
    uint32_t last_log = 0;
    motor_state_t previous = MOTOR_STOPPED;
    while(1){
        uint32_t now = timer_now_ms();
        motor_update(now);
        if(command_receive(line)){ command_motor_execute(line); }
        demo_update(timer_now_ms());
        motor_state_t state = motor_get_state();
        if(state != previous){
            previous = state;
            command_motor_send_status();
        }
        if(command_motor_log_enabled() && now - last_log >= 200UL){
            last_log = now;
            usart0_send_string("LOG,"); usart0_send_uint32(now);
            usart0_send_string(","); usart0_send_uint32(motor_get_current_hz());
            usart0_send_newline();
        }
    }
}
