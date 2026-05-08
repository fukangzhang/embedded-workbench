#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t count)
{
    unsigned char *dst = (unsigned char *)destination;
    const unsigned char *src = (const unsigned char *)source;
    size_t index = 0u;

    for (index = 0u; index < count; index++) {
        dst[index] = src[index];
    }

    return destination;
}

void *memset(void *destination, int value, size_t count)
{
    unsigned char *dst = (unsigned char *)destination;
    size_t index = 0u;

    for (index = 0u; index < count; index++) {
        dst[index] = (unsigned char)value;
    }

    return destination;
}

void *memmove(void *destination, const void *source, size_t count)
{
    unsigned char *dst = (unsigned char *)destination;
    const unsigned char *src = (const unsigned char *)source;

    if (dst < src) {
        size_t index = 0u;

        for (index = 0u; index < count; index++) {
            dst[index] = src[index];
        }
    } else if (dst > src) {
        while (count > 0u) {
            count--;
            dst[count] = src[count];
        }
    }

    return destination;
}
