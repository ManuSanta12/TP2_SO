#include <stdint.h>
#include "time.h"

static unsigned long ticks = 0;
extern int _hlt();

void timer_handler() {
	ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}


int ms_elapsed() {
    return ticks*5000/91;
}

