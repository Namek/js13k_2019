#ifndef COMMON_H
#define COMMON_H

#ifndef WIN32
typedef unsigned long size_t;
#endif
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
#define NULL 0

#include "utils/memory.hpp"

#ifdef WIN32
#define WASM_EXPORT
#else
#define WASM_EXPORT __attribute__((used))
#endif
#define Ref auto &

#ifndef WIN32
extern "C" {
#endif
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

#ifdef WIN32
#include <stdlib.h>
#include <cmath>
#define randomf() ((float)rand() / (float)RAND_MAX)
#define abs(a) fabs(a)
#else
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
#endif

#ifndef WIN32
}
#endif

#endif
