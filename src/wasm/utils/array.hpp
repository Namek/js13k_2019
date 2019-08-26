#ifndef ARRAY_H
#define ARRAY_H

#include "memory.hpp"
#include "../common.hpp"

#define block_size 1024

struct Array_Block {
  int elementCount, removedElsCount;
  int elementSize;
  char *data; // it's 1-byte type so pointer arithmetic is possible

  void init(int elementSize) {
    elementCount = 0;
    removedElsCount = 0;
    this->elementSize = elementSize;
    data = (char *)malloc(elementSize * block_size);
  }
};

// why is this a macro rather than a function? Because compiler somehow finds out that's
// a good replacement for memset() which we don't want to additionally implement in JavaScript.
#define MEMZERO(ptr, size)       \
  for (int i = 0; i < size; ++i) \
    ptr[i] = 0;

// This structure is best to be used exclusively in one of two ways:
// 1. only set/get/createAt - when the size doesn't matter and you don't watch out for holes made after removals
// 2. create/get/remove - when some random elements are removed and you want to preserve memory
struct Array {
  Array_Block *blocks;
  int blocks_reserved;
  int elementSize;

  public:
  uint size;

  // TODO implement dynamic resize and handle holes after removed elements

  void init(int elementSize) {
    this->size = 0;
    this->elementSize = elementSize;
    this->blocks_reserved = 1;
    blocks = (Array_Block *)malloc(sizeof(Array_Block) * blocks_reserved);

    for (int i = 0; i < blocks_reserved; ++i)
      blocks[i].init(elementSize);
  }

  char *getPointer(int index) {
    // TODO check size of blocks, use other block if needed
    return blocks[0].data + index * elementSize;
  }

  void set(int index, void *el) {
    char *ptr = getPointer(index);
    *ptr = *((char *)el);
  }

  char *createPtr() {
    // TODO support holes after removals
    char *ptr = getPointer(size);
    size += 1;
    return ptr;
  }

  char *createPtrWithIndex(int *index) {
    *index = size;
    return createPtr();
  }

  // mix of createPtr() and set() - makes sure that index exists but does not move the `size` value
  char *createPtrAt(int index) {
    // TODO support holes after removals - make sure the memory is allocated
    char *ptr = getPointer(index);
    return ptr;
  }

  void remove(int index) {
    // TODO we do nothing special until holes are not saved for memory reusage on creation
    // thus, we will not modify 'size' so new elements don't overwrite existing ones
    char *ptr = getPointer(index);
    MEMZERO(ptr, elementSize);
  }

  char *getPtr(int index) {
    return getPointer(index);
  }
};

#endif