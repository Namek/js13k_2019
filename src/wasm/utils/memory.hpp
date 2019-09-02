#ifndef MEMORY__H
#define MEMORY__H

#include "../common.hpp"

extern "C" {
void *malloc(size_t size);
void free(void *ptr);
void memcpy(void *dst, const void *src, int len);
void memset(void *pointer, uchar value, uint size);

extern size_t sbrk(size_t);
}

#define Allocate(type, count) (type *)malloc(sizeof(type) * count)

#endif
