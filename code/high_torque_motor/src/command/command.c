#include "command/command.h"
#include "usart.h"
#include <string.h>

bool command_receive(char line[COMMAND_LEN])
{
    static char buffer[COMMAND_LEN];
    static uint8_t length;
    static bool discard;
    while(usart0_rx_ready()){
        if(usart0_take_rx_error()){
            length = 0;
            discard = true;
            usart0_send_string("ERROR serial overflow; press Enter, then retry\r\n");
            return false;
        }
        uint8_t c = usart0_rx_read();
        if(c == '\r' || c == '\n'){
            bool ready = !discard && length != 0;
            if(ready){
                buffer[length] = '\0';
                memcpy(line, buffer, length + 1U);
            } else if(discard){
                usart0_send_string("ERROR line discarded; retry command\r\n");
            }
            length = 0;
            discard = false;
            if(ready){ return true; }
        } else if(!discard){
            if(c == 8U || c == 127U){ if(length){ length--; } }
            else if(c < 32U || c > 126U){ discard = true; }
            else if(length < COMMAND_LEN - 1U){ buffer[length++] = (char)c; }
            else { discard = true; }
        }
    }
    return false;
}

bool command_parse_uint32(const char *text, uint32_t *value)
{
    if(*text == '\0'){ return false; }
    uint32_t number = 0;
    while(*text){
        if(*text < '0' || *text > '9'){ return false; }
        uint8_t digit = (uint8_t)(*text++ - '0');
        if(number > (UINT32_MAX - digit) / 10UL){ return false; }
        number = number * 10UL + digit;
    }
    *value = number;
    return true;
}
