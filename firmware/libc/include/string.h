#ifndef EW_FREESTANDING_STRING_H
#define EW_FREESTANDING_STRING_H

#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t count);
void *memset(void *destination, int value, size_t count);
void *memmove(void *destination, const void *source, size_t count);

#endif
