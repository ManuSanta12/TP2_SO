#include <scheduler.h>
#include <timeRtc.h>

extern void getTime(uint8_t *h, uint8_t *m, uint8_t *s);

static uint32_t get_seconds() {
	uint8_t h, m, s;
	getTime(&h, &m, &s);
	return s + m * 60 + ((h + 24 - 3) % 24) * 3600;
}

void sleep_time(int seconds) {
	uint32_t limit = get_seconds() + seconds;
	while (get_seconds() < limit)
		yield();
}