#include "command/command_motor.h"
#include "command/command.h"
#include "demo.h"
#include "usart.h"
#include <string.h>

static bool log_enabled;
void command_motor_send_state(motor_state_t state)
{
    switch(state){
        case MOTOR_STOPPED: usart0_send_string("STOPPED"); break;
        case MOTOR_ACCEL: usart0_send_string("ACCEL"); break;
        case MOTOR_CRUISE: usart0_send_string("CRUISE"); break;
        case MOTOR_DECEL: usart0_send_string("DECEL"); break;
        case MOTOR_STOPPING: usart0_send_string("STOPPING"); break;
        case MOTOR_REVERSING: usart0_send_string("REVERSING"); break;
    }
}
void command_motor_send_status(void)
{
    usart0_send_string("STATE: ");
    command_motor_send_state(motor_get_state());
    usart0_send_string(", HZ: "); usart0_send_uint32(motor_get_current_hz());
    usart0_send_string(", TARGET: "); usart0_send_uint32(motor_get_target_hz());
    usart0_send_string(", ACCEL: "); usart0_send_uint32(motor_get_accel());
    usart0_send_string(", DIR: "); usart0_send_uint32(motor_get_direction());
    usart0_send_newline();
}
bool command_motor_log_enabled(void){ return log_enabled; }

void command_motor_execute(const char *line)
{
    uint32_t number;
    if(strcmp(line, "help") == 0){
        usart0_send_string("start / stop / estop / reverse / status\r\n"
            "speed <Hz>: 50-2000; default 400\r\n"
            "accel <Hz/s>: 1-2000; low=100, high=500; default 200\r\n"
            "demo: ramp up, cruise 3 seconds, ramp down, stop\r\n"
            "log on / log off: commanded Hz every 200 ms\r\n"
            "PPR=1600; RPM=Hz*60/1600. No motor feedback connected.\r\n");
        return;
    } else if(strcmp(line, "status") == 0){
        command_motor_send_status(); return;
    } else if(strcmp(line, "log on") == 0){ log_enabled = true; }
    else if(strcmp(line, "log off") == 0){ log_enabled = false; }
    else if(strcmp(line, "demo") == 0){
        if(!demo_start()){
            usart0_send_string("ERROR stop motor before demo\r\n"); return;
        }
    } else if(strcmp(line, "start") == 0){ demo_cancel(); motor_start(); }
    else if(strcmp(line, "stop") == 0){ demo_cancel(); motor_stop(); }
    else if(strcmp(line, "estop") == 0){ demo_cancel(); motor_estop(); }
    else if(strcmp(line, "reverse") == 0){ demo_cancel(); motor_reverse(); }
    else if(strncmp(line, "speed ", 6) == 0){
        if(!command_parse_uint32(line + 6, &number) || !motor_set_target(number)){
            usart0_send_string("ERROR speed must be 50-2000 Hz\r\n"); return;
        }
        demo_cancel();
    } else if(strncmp(line, "accel ", 6) == 0){
        if(strcmp(line + 6, "low") == 0){ number = MOTOR_ACCEL_LOW; }
        else if(strcmp(line + 6, "high") == 0){ number = MOTOR_ACCEL_HIGH; }
        else if(!command_parse_uint32(line + 6, &number)){
            usart0_send_string("ERROR invalid acceleration\r\n"); return;
        }
        if(!motor_set_accel(number)){
            usart0_send_string("ERROR accel must be 1-2000 Hz/s; zero disabled\r\n"); return;
        }
        demo_cancel();
    } else {
        usart0_send_string("ERROR unknown command; type help\r\n"); return;
    }
    usart0_send_string("OK "); usart0_send_string(line); usart0_send_newline();
}
