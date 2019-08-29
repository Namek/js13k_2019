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
#ifndef PRODUCTION
extern void _abort(const char *text);
extern void _l(int);    //logging int
extern void _lf(float); //logging float
extern void _lstr(const char *text, int num);
extern void _lfstr(const char *text, float num);
#else
#define _abort(_) {};
#define _l(_) {};
#define _lf(_) {};
#define _lstr(_, __) {};
#define _lfstr(_, __) {};
#endif

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
