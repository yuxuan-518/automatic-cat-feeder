#ifndef COMMAND_H
#define COMMAND_H
#include <stdbool.h>
#include <stdint.h>
#define COMMAND_LEN 48
bool command_receive(char line[COMMAND_LEN]);
bool command_parse_uint32(const char *text, uint32_t *value);
#endif
