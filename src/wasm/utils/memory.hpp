#ifndef MEMORY__H
#define MEMORY__H

#include <stdlib.h>

extern "C" {
void *malloc(size_t size);
void free(void *ptr);

extern size_t sbrk(size_t);
}

#endif
