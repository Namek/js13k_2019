#ifndef COMMON_H
#define COMMON_H


typedef unsigned long size_t;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#define NULL 0

#include "utils/memory.hpp"

#define WASM_EXPORT __attribute__((used))
#define ref auto &

extern "C" {
extern void _l(int);     //logging
extern void _lstr(const char* text, int num);
extern float randomf(); // returns 0..1 float. 'f' suffix is here because of collision with stdlib function

extern int round(float);
extern int floor(float);
extern int ceil(float);
extern float cos(float);
extern float acos(float);
extern float sin(float);
extern float tan(float);
extern float sqrt(float);
extern float abs(float);
extern float pow(float, float);
}


#endif
