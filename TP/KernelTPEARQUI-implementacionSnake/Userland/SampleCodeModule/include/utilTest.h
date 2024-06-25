#ifndef UTILTEST_H
#define UTILTEST_H

#include <stdint.h>
#include <usr_stdlib.h>
#include <uniqueTypes.h>

static uint32_t m_z;
static uint32_t m_w;

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
uint8_t memcheck(void *start, uint8_t value, uint32_t size);
size_t strlen_custom(const char* str);
int64_t satoi(char *str);
void bussy_wait(uint64_t n);
void endless_loop();
void endless_loop_print(uint64_t wait);

#endif 