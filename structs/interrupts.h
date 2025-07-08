// structs/interrupts.h
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include "registers.h"

void register_interrupt_handler(int n, void (*handler)(struct registers*));

#endif

