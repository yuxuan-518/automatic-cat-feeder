/* Host tests substitute GPIO/UART only; exercise the actual controller code. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "motor.h"
#include "motor_io.h"
#include "demo.h"
#include "usart.h"
#include "command/command.h"
#include "command/command_motor.h"

static bool running, rx_error;
static uint8_t direction;
static uint32_t output_hz, now;
static const char *input = "";
static char output[4096];
static size_t output_length;

void motor_io_init(void){ running = false; direction = 0; output_hz = 0; }
void motor_io_start(uint32_t hz){ running = true; output_hz = hz; }
void motor_io_set_hz(uint32_t hz){ output_hz = hz; }
void motor_io_stop(void){ running = false; output_hz = 0; }
void motor_io_flip_direction(void){ assert(!running); direction ^= 1U; }
uint8_t motor_io_direction(void){ return direction; }
bool usart0_rx_ready(void){ return *input != '\0'; }
uint8_t usart0_rx_read(void){ return (uint8_t)*input++; }
bool usart0_take_rx_error(void){ bool value = rx_error; rx_error = false; return value; }
void usart0_send_string(const char *text)
{
    size_t length = strlen(text);
    assert(output_length + length < sizeof(output));
    memcpy(output + output_length, text, length + 1);
    output_length += length;
}
void usart0_send_newline(void){ usart0_send_string("\r\n"); }
void usart0_send_uint32(uint32_t value)
{
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)value);
    usart0_send_string(buffer);
}

static void reset(void)
{
    demo_cancel(); motor_init(); now = 0;
    output_length = 0; output[0] = '\0';
}
static void advance(uint32_t milliseconds)
{
    assert(milliseconds % 10UL == 0);
    for(uint32_t elapsed = 0; elapsed < milliseconds; elapsed += 10){
        now += 10; motor_update(now); demo_update(now);
    }
}
static bool receive(const char *text, char *line)
{
    input = text;
    return command_receive(line);
}

int main(void)
{
    reset();
    assert(!running && motor_get_current_hz() == 0);
    assert(motor_get_target_hz() == 400 && motor_get_accel() == 200);
    command_motor_execute("start");
    assert(running && output_hz == 50);
    advance(1000); assert(motor_get_current_hz() == 250);
    advance(750); assert(motor_get_state() == MOTOR_CRUISE && output_hz == 400);
    command_motor_execute("speed 200");
    assert(motor_get_state() == MOTOR_DECEL);
    advance(1000); assert(output_hz == 200);
    command_motor_execute("stop");
    advance(750); assert(!running && motor_get_state() == MOTOR_STOPPED);

    reset(); motor_start(); advance(2000);
    motor_reverse(); assert(motor_get_state() == MOTOR_REVERSING);
    advance(1740); assert(direction == 0 && running);
    advance(10); assert(direction == 1 && running && output_hz == 50);
    advance(1750); assert(output_hz == 400);
    motor_reverse(); advance(100); motor_stop(); advance(2000);
    assert(!running && direction == 1); /* stop cancels queued reversal */
    motor_start(); advance(1750); motor_reverse(); motor_reverse();
    assert(motor_get_state() == MOTOR_CRUISE && direction == 1);
    motor_estop(); assert(!running && motor_get_current_hz() == 0);
    advance(5000); assert(!running);
    motor_reverse(); assert(!running && direction == 0);

    reset();
    assert(!motor_set_accel(0) && !motor_set_accel(2001));
    assert(!motor_set_target(49) && !motor_set_target(2001));
    assert(motor_set_accel(1)); motor_start();
    advance(1000); assert(output_hz == 51); /* fractional ramp increments */
    motor_update(now + 10000); assert(output_hz == 51); /* no catch-up jump */

    reset();
    now = UINT32_MAX - 19UL;
    motor_update(now); motor_start();
    advance(1000); assert(output_hz == 250); /* millisecond clock wrap */

    reset();
    assert(demo_start()); assert(!demo_start());
    advance(6500);
    assert(!running && motor_get_state() == MOTOR_STOPPED);
    assert(strstr(output, "DEMO DONE") != NULL);
    reset(); command_motor_execute("demo"); advance(200);
    command_motor_execute("estop"); advance(10000);
    assert(!running && strstr(output, "DEMO DONE") == NULL);

    uint32_t value = 123;
    assert(command_parse_uint32("4294967295", &value) && value == UINT32_MAX);
    assert(!command_parse_uint32("4294967296", &value));
    assert(!command_parse_uint32("429496729600000000", &value));
    assert(!command_parse_uint32("-1", &value));
    assert(!command_parse_uint32("400junk", &value));
    assert(!command_parse_uint32("", &value));
    reset();
    command_motor_execute("speed 4294967696");
    command_motor_execute("accel 0");
    command_motor_execute("speed -1");
    command_motor_execute("startjunk");
    assert(!running && motor_get_target_hz() == 400 && motor_get_accel() == 200);

    char line[COMMAND_LEN];
    assert(receive("start\r\n", line) && strcmp(line, "start") == 0);
    assert(!command_receive(line)); /* CRLF is one command */
    assert(receive("stop\r", line) && strcmp(line, "stop") == 0);
    assert(receive("status\n", line) && strcmp(line, "status") == 0);
    assert(receive("starX\b t\b\bt\n", line) && strcmp(line, "start") == 0);
    char long_line[100]; memset(long_line, 'x', sizeof(long_line));
    long_line[98] = '\n'; long_line[99] = '\0';
    assert(!receive(long_line, line));
    assert(receive("status\n", line) && strcmp(line, "status") == 0);
    assert(!receive("start\x01\n", line));
    rx_error = true;
    assert(!receive("start\n", line));
    assert(!command_receive(line)); /* discard corrupted line, including newline */
    assert(receive("status\n", line));
    puts("PASS: ramp, stop, reversal, estop, demo, clock wrap, numeric validation, serial framing/recovery");
    return 0;
}
