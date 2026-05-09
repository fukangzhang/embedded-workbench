#ifndef EW_FREESTANDING_STRING_H
#define EW_FREESTANDING_STRING_H

#include <stddef.h>

/* 裸机 freestanding 环境不保证有完整标准库；启动代码和编译器仍可能需要这些符号。 */
void *memcpy(void *destination, const void *source, size_t count);
void *memset(void *destination, int value, size_t count);
/* memmove 与 memcpy 的关键差异是：memmove 必须能处理源和目标内存区域重叠。 */
void *memmove(void *destination, const void *source, size_t count);

#endif
