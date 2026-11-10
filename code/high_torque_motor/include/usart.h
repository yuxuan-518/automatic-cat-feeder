#ifndef USART_H
#define USART_H
#include <stdbool.h>
#include <stdint.h>
void usart0_init(void);
bool usart0_rx_ready(void);
uint8_t usart0_rx_read(void);
bool usart0_take_rx_error(void);
void usart0_send_string(const char *text);
void usart0_send_uint32(uint32_t number);
void usart0_send_newline(void);
#endif
