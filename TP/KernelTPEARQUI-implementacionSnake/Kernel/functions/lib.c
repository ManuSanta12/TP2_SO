#include <stdint.h>
#include <lib.h>
#include <memoryManager.h>

void * memset(void * destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destination;

	while(length--)
		dst[length] = chr;

	return destination;
}

void * memcpy(void * destination, const void * source, uint64_t length)
{
	/*
	* memcpy does not support overlapping buffers, so always do it
	* forwards. (Don't change this without adjusting memmove.)
	*
	* For speedy copying, optimize the common case where both pointers
	* and the length are word-aligned, and copy word-at-a-time instead
	* of byte-at-a-time. Otherwise, copy by bytes.
	*
	* The alignment logic below should be portable. We rely on
	* the compiler to be reasonably intelligent about optimizing
	* the divides and modulos out. Fortunately, it is.
	*/
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *) destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t * d = (uint8_t*)destination;
		const uint8_t * s = (const uint8_t*)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

unsigned int log(uint64_t n, int base) {
    unsigned int count = 0;
    double result = (double)n;
    while (result > 1) {
        result /= base;
        count++;
    }
    if (n > base && n % base != 0) {
        count++;
    }
    return count;
}

unsigned int strlen(const char *str) {
    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char * strcpy(const char * str) {
	unsigned int len = strlen(str);
	char * new_str = memory_manager_malloc(len+1);
	if (new_str != NULL) {
		memcpy(new_str, str, len+1);
	}
	return new_str;
}

int strcmp(const char *str1, const char *str2){
	int i = 0;
	int ret = 0;

	while (str1[i] != 0 && str2[i] != 0){
		if (str1[i] - str2[i] != 0){
			return str1[i] - str2[i];
		}
		i++;
	}

	if (str1[i] != 0 || str2[i] != 0){
		return str1[i] - str2[i];
	}

	return ret;
}
