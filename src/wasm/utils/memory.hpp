#ifndef MEMORY__H
#define MEMORY__H

#include "../common.hpp"

extern "C" {
void *malloc(size_t size);
void free(void *ptr);
void memcpy(void *src, void *dst, int len);

extern size_t sbrk(size_t);
}

#endif
