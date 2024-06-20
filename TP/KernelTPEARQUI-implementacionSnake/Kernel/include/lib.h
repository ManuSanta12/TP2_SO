#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);
unsigned int log(uint64_t n, int base);

char *cpuVendor(char *result);
uint64_t getSeconds();
uint64_t getMinutes();
uint64_t getHours();
int strcmp(const char *str1, const char *str2);
unsigned int strlen(const char *str);
char * strcpy(const char * str);

#endif