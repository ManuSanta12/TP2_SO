#ifndef IRQDISPATCHER_H
#define IRQDISPATCHER_H
#include <stdint.h>
#include <time.h>
#include <keyboard.h>
#include <stdint.h>

void irqDispatcher(uint64_t irq, uint64_t * registers);

#endif