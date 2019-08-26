#include "common.h"

float toRadian(float a) {
  return a * degree;
};

float mathTmp[VEC_SIZE_3 + VEC_SIZE_4 + MAT_SIZE_4];

int CLAMP255(int a) {
  return (int)CLAMP(a, 0, 255);
}

float CLAMP01(float a) {
  return CLAMP(a, 0.0f, 1.0f);
}

float LERP(float alpha, float from, float to) {
  return from * (1.0f - alpha) + (float)to * alpha;
}