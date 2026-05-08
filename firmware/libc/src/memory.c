#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t count)
{
    unsigned char *dst = (unsigned char *)destination;
    const unsigned char *src = (const unsigned char *)source;
    size_t index = 0u;

    /* freestanding 固件环境下可能没有完整 libc，这里提供最小可链接实现。 */
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

    /* memmove 需要处理重叠区域：目标在源前面时正向拷贝，目标在源后面时反向拷贝。 */
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

void __aeabi_memclr(void *destination, size_t count)
{
    (void)memset(destination, 0, count);
}

void __aeabi_memclr4(void *destination, size_t count)
{
    __aeabi_memclr(destination, count);
}

void __aeabi_memclr8(void *destination, size_t count)
{
    __aeabi_memclr(destination, count);
}
