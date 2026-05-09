#ifndef EW_FREESTANDING_STDLIB_H
#define EW_FREESTANDING_STDLIB_H

#include <stddef.h>

#ifndef NULL
/* 有些 freestanding 工具链头文件不提供 NULL，这里补一个最小定义。 */
#define NULL ((void *)0)
#endif

#endif
