#ifndef MEMORY__H
#define MEMORY__H

#include "../common.hpp"

extern "C" {
void *malloc(size_t size);
void free(void *ptr);
void memcpy(void* dst, const void* src, int len);

extern size_t sbrk(size_t);
}

#define Allocate(type, count) (type *)malloc(sizeof(type) * count)

#endif
