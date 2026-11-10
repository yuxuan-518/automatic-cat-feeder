#include "usart.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define RX_SIZE 64U
static volatile uint8_t rx_buffer[RX_SIZE], head, tail;
static volatile bool rx_error;

ISR(USART0_RX_vect)
{
    uint8_t status = UCSR0A;
    uint8_t data = UDR0;
    uint8_t next = (head + 1U) & (RX_SIZE - 1U);
    if((status & (_BV(FE0) | _BV(DOR0) | _BV(UPE0))) || next == tail){
        rx_error = true;
    } else {
        rx_buffer[head] = data;
        head = next;
    }
}

void usart0_init(void)
{
    UCSR0A = 0;
    UBRR0H = 0;
    UBRR0L = (uint8_t)(F_CPU / 16UL / 9600UL - 1UL);
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

bool usart0_rx_ready(void){ return head != tail; }
uint8_t usart0_rx_read(void)
{
    uint8_t data = rx_buffer[tail];
    tail = (tail + 1U) & (RX_SIZE - 1U);
    return data;
}

bool usart0_take_rx_error(void)
{
    bool error;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        error = rx_error;
        if(error){ tail = head; rx_error = false; }
    }
    return error;
}

static void send_byte(uint8_t data)
{
    while(!(UCSR0A & _BV(UDRE0))){}
    UDR0 = data;
}

void usart0_send_string(const char *text){ while(*text){ send_byte(*text++); } }
void usart0_send_newline(void){ usart0_send_string("\r\n"); }
void usart0_send_uint32(uint32_t number)
{
    char digits[10];
    uint8_t count = 0;
    do { digits[count++] = '0' + number % 10; number /= 10; } while(number);
    while(count){ send_byte(digits[--count]); }
}
