#ifndef DEMO_H
#define DEMO_H
#include <stdbool.h>
#include <stdint.h>
bool demo_start(void);
void demo_cancel(void);
void demo_update(uint32_t now);
#endif
