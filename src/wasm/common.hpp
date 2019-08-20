#ifndef COMMON_H
#define COMMON_H

#define WASM_EXPORT __attribute__((used))

extern "C" {
extern void _l(int);     //logging
extern void _lstr(const char* text, int num);
extern float randomf(); // returns 0..1 float. 'f' suffix is here because of collision with stdlib function
}

#endif
