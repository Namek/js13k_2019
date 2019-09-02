#include "memory.hpp"
#include "../common.hpp"

// this implementation is based on https://github.com/arjun024/memalloc
// and applies following modifications:
//  - only malloc() and free() are used, calloc() and realloc() were not useful
//  - removed thread synchronisation
//  - implemented sbrk() in JavaScript by using WebAssembly grow() and drop()

typedef char ALIGN[16];

union header
{
  struct {
    size_t size;
    unsigned is_free;
    union header *next;
  } s;
  ALIGN stub;
};
typedef union header header_t;

header_t *head = NULL, *tail = NULL;

header_t *get_free_block(size_t size) {
  header_t *curr = head;
  while (curr) {
    if (curr->s.is_free && curr->s.size >= size)
      return curr;
    curr = curr->s.next;
  }
  return NULL;
}

void *malloc(size_t size) {
  size_t total_size;
  void *block;
  header_t *header;
  if (!size)
    return NULL;

  header = get_free_block(size);
  if (header) {
    header->s.is_free = 0;
    return (void *)(header + 1);
  }
  total_size = sizeof(header_t) + size;
  block = (header_t *)sbrk(total_size);
  if (block == (void *)-1) {
    return NULL;
  }
  header = (header_t *)block;
  header->s.size = size;
  header->s.is_free = 0;
  header->s.next = NULL;

  if (!head)
    head = header;
  if (tail)
    tail->s.next = header;
  tail = header;
  return (void *)(header + 1);
}

void free(void *block) {
  header_t *header, *tmp;
  void *programbreak;

  if (!block)
    return;
  header = (header_t *)block - 1;

  programbreak = (void *)sbrk(0);
  if ((char *)block + header->s.size == programbreak) {
    if (head == tail) {
      head = NULL;
      tail = NULL;
    }
    else {
      tmp = head;
      while (tmp) {
        if (tmp->s.next == tail) {
          tmp->s.next = NULL;
          tail = tmp;
        }
        tmp = tmp->s.next;
      }
    }
    sbrk(0 - sizeof(header_t) - header->s.size);
    return;
  }
  header->s.is_free = 1;
}

void memcpy(void* dst, const void* src, int len) {
  char* s = (char*)src;
  char* d = (char*)dst;

  for (uint i = 0; i < len; ++i) {
    d[i] = s[i];
  }
}

void memset(void* pointer, uchar value, uint size) {
  auto ptr = (uchar*)pointer;
  auto endPtr = ptr + size;

  while (ptr++ != endPtr) {
    *ptr = value;
  }
}