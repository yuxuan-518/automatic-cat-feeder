#include "timer.h"
#include <avr/io.h>

#define TIMER_PRESCALER 64UL

static bool calc_ocr(uint32_t hz, uint8_t edges, uint16_t *ocr)
{
    /* 先检查范围，避免参数乘法溢出。STEP 每两个翻转产生一个周期。 */
    if(hz == 0 || hz > F_CPU / (TIMER_PRESCALER * edges)){
        return false;
    }
    uint32_t ticks = F_CPU / (TIMER_PRESCALER * edges * hz);
    if(ticks == 0 || ticks > 65536UL){
        return false;
    }
    *ocr = (uint16_t)(ticks - 1UL);
    return true;
}

bool step_timer_set_hz(uint8_t id, uint32_t hz)
{
    uint16_t ocr;
    if(id >= 2 || !calc_ocr(hz, 2, &ocr)){
        return false;
    }
    /* 调用者先停车，避免降低 OCR 时错过比较点。 */
    if(id == 0){
        OCR1A = ocr;
    } else {
        OCR4A = ocr;
    }
    return true;
}

bool step_timer_init(uint8_t id, uint32_t hz)
{
    if(id >= 2){
        return false;
    }
    if(id == 0){
        TCCR1A = (1 << COM1A0);
        TCCR1B = (1 << WGM12); /* CTC，尚未启动时钟。 */
        TCCR1C = 0;
        TIMSK1 = 0;
        TCNT1 = 0;
    } else {
        TCCR4A = (1 << COM4A0);
        TCCR4B = (1 << WGM42);
        TCCR4C = 0;
        TIMSK4 = 0;
        TCNT4 = 0;
    }
    return step_timer_set_hz(id, hz);
}

bool step_timer_is_running(uint8_t id)
{
    if(id == 0){
        return (TCCR1B & ((1 << CS12) | (1 << CS11) | (1 << CS10))) != 0;
    }
    if(id == 1){
        return (TCCR4B & ((1 << CS42) | (1 << CS41) | (1 << CS40))) != 0;
    }
    return false;
}

void step_timer_start(uint8_t id)
{
    if(step_timer_is_running(id)){
        return;
    }
    if(id == 0){
        TCNT1 = 0;
        TCCR1B |= (1 << CS11) | (1 << CS10);
    } else if(id == 1){
        TCNT4 = 0;
        TCCR4B |= (1 << CS41) | (1 << CS40);
    }
}

void step_timer_stop(uint8_t id)
{
    if(id == 0){
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        TCNT1 = 0;
    } else if(id == 1){
        TCCR4B &= ~((1 << CS42) | (1 << CS41) | (1 << CS40));
        TCNT4 = 0;
    }
}

bool timer3_init(uint32_t hz)
{
    uint16_t ocr;
    if(!calc_ocr(hz, 1, &ocr)){
        return false;
    }
    TCCR3A = 0;
    TCCR3B = (1 << WGM32);
    TCNT3 = 0;
    OCR3A = ocr;
    TIMSK3 = (1 << OCIE3A);
    TCCR3B |= (1 << CS31) | (1 << CS30);
    return true;
}
