#include "demo.h"
#include "motor.h"
#include "usart.h"

static uint8_t phase;
static uint32_t cruise_start;
bool demo_start(void)
{
    if(phase || motor_get_state() != MOTOR_STOPPED){ return false; }
    motor_start();
    phase = 1;
    return true;
}
void demo_cancel(void){ phase = 0; }
void demo_update(uint32_t now)
{
    motor_state_t state = motor_get_state();
    if(phase == 1 && state == MOTOR_CRUISE){ cruise_start = now; phase = 2; }
    else if(phase == 2 && now - cruise_start >= 3000UL){ motor_stop(); phase = 3; }
    else if(phase == 3 && state == MOTOR_STOPPED){
        phase = 0;
        usart0_send_string("DEMO DONE\r\n");
    }
}
