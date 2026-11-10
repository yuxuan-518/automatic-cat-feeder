#include "command/command_motor.h"
#include "command/command.h"
#include "motor.h"
#include "usart.h"
#include <string.h>

static void send_one_status(motor_id_t id)
{
    usart0_send_string("MOTOR ");
    usart0_send_uint32((uint8_t)id + 1);
    usart0_send_string(motor_is_running(id) ? ": ON, SPEED: " : ": OFF, SPEED: ");
    usart0_send_uint32(motor_get_speed(id));
    usart0_send_string(" Hz, DIR: ");
    usart0_send_uint32(motor_get_direction(id));
    usart0_send_newline();
}

void command_motor_send_status(void)
{
    send_one_status(MOTOR_1);
    send_one_status(MOTOR_2);
}

static void send_help(void)
{
    usart0_send_string(
        "start / stop / reverse / status: both motors\r\n"
        "start 1 / start 2: start one motor\r\n"
        "stop 1 / stop 2: stop one motor\r\n"
        "reverse 1 / reverse 2: reverse one motor\r\n"
        "status 1 / status 2: show one motor\r\n"
        "speed <Hz>: set both (50-8000)\r\n"
        "speed 1 <Hz> / speed 2 <Hz>: set one\r\n"
        "M1 STEP D11, DIR D12; M2 STEP D6, DIR D7\r\n");
}

static bool select_motor(const char *arg, uint8_t *first, uint8_t *end)
{
    if(strcmp(arg, "1") != 0 && strcmp(arg, "2") != 0){
        usart0_send_string("WARNING motor must be 1 or 2\r\n");
        return false;
    }
    *first = (uint8_t)(arg[0] - '1');
    *end = *first + 1;
    return true;
}

void command_motor_execute(const char line[])
{
    char buffer[COMMAND_LEN];
    char *args[3];
    uint8_t count = 0;
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    char *token = strtok(buffer, " \t");
    while(token != 0){
        if(count >= 3){
            usart0_send_string("WARNING too many arguments\r\n");
            return;
        }
        args[count++] = token;
        token = strtok(0, " \t");
    }
    if(count == 0){
        return;
    }
    if(strcmp(args[0], "help") == 0 && count == 1){
        send_help();
        return;
    }

    uint8_t first = 0;
    uint8_t end = MOTOR_COUNT;
    if(strcmp(args[0], "speed") == 0){
        if(count != 2 && count != 3){
            usart0_send_string("WARNING use speed [1|2] <Hz>\r\n");
            return;
        }
        if(count == 3 && !select_motor(args[1], &first, &end)){
            return;
        }
        uint32_t hz = command_parse_uint32(args[count - 1]);
        if(hz < MOTOR_MIN_HZ || hz > MOTOR_MAX_HZ){
            usart0_send_string("WARNING speed must be 50-8000 Hz\r\n");
            return;
        }
        for(uint8_t i = first; i < end; i++){
            if(!motor_set_speed((motor_id_t)i, hz)){
                usart0_send_string("WARNING speed update failed\r\n");
                return;
            }
        }
    } else {
        bool start = strcmp(args[0], "start") == 0;
        bool stop = strcmp(args[0], "stop") == 0;
        bool reverse = strcmp(args[0], "reverse") == 0;
        bool status = strcmp(args[0], "status") == 0;
        if(!(start || stop || reverse || status) || count > 2){
            usart0_send_string("WARNING unknown command, type help\r\n");
            return;
        }
        if(count == 2 && !select_motor(args[1], &first, &end)){
            return;
        }
        for(uint8_t i = first; i < end; i++){
            motor_id_t id = (motor_id_t)i;
            if(start){
                motor_start(id);
            } else if(stop){
                motor_stop(id);
            } else if(reverse){
                motor_change_direction(id);
            } else {
                send_one_status(id);
            }
        }
        if(status){
            return;
        }
    }
    usart0_send_string("OK ");
    usart0_send_string(line);
    usart0_send_newline();
}
