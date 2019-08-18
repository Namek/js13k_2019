#ifndef ARRAY_H
#define ARRAY_H

template <class T, unsigned int block_size>
struct Array_Block {
  T data[block_size];
  int elementCount, removedElsCount;

  Array_Block() : elementCount(0), removedElsCount(0) {}
};

template <class T, unsigned int block_size = 1024>
class Array {
  Array_Block<T, block_size> firstBlock;
  Array_Block<T, block_size> *nextBlocks;

  // TODO implement dynamic resize and handle holes after removed elements

  public:
  int size;

  Array() : size(0) {}

  void set(int index, T &el) {
    firstBlock.data[index] = *el;
  }

  int add(T &el) {
    firstBlock.elementCount++;
    firstBlock.data[size++] = *el;
    return size;
  }

  int add(T el) {
    firstBlock.elementCount++;
    firstBlock.data[size++] = el;
    return size;
  }

  T &create() {
    firstBlock.elementCount++;
    return firstBlock.data[size++];
  }

  // does not manage size
  T &createAt(unsigned int index) {
    return firstBlock.data[index];
  }

  inline T &operator[](int index) {
    return firstBlock.data[index];
  }
};

#endif