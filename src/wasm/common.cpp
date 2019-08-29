#include "common.hpp"

#ifdef PRODUCTION
void _abort(const char *text) {}
void _l(int) {}
void _lf(float) {}
void _lstr(const char *text, int num) {}
void _lfstr(const char *text, float num) {}
#endif