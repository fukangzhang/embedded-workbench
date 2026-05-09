#include <stddef.h>

void *memcpy(void *destination, const void *source, size_t count)
{
    /* 用 unsigned char 逐字节访问内存，这是 C 里实现通用内存复制的常见做法。 */
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

    /* 标准 memset 接收 int，但真正写入内存的是低 8 bit。 */
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
    /* ARM EABI 可能让编译器生成 __aeabi_memclr 调用，用来把一段内存清零。 */
    (void)memset(destination, 0, count);
}

void __aeabi_memclr4(void *destination, size_t count)
{
    /* 4/8 后缀通常表示调用方认为地址或长度有对齐假设；当前实现复用通用清零即可。 */
    __aeabi_memclr(destination, count);
}

void __aeabi_memclr8(void *destination, size_t count)
{
    __aeabi_memclr(destination, count);
}
