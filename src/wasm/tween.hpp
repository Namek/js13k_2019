#ifndef TWEEN__HPP
#define TWEEN__HPP

#include "utils/array.hpp"

enum TweenDataType
{
  TweenInt,
  TweenIntPtr,
  TweenFloat,
  TweenFloatPtr
};

enum TweenProgressType
{
  Linear,
  Sin,
  Cos
};

union TweenValue
{
  int intVal;
  int *intPtr;
  float floatVal;
  float *floatPtr;

  TweenValue() = default;

  TweenValue(float v) {
    floatVal = v;
  }
  TweenValue(int v) {
    intVal = v;
  }
  TweenValue(float *p) {
    floatPtr = p;
  }
  TweenValue(int *p) {
    intPtr = p;
  }
};

struct Tween {
  bool active;
  float progress;

  float duration;
  void *target;
  TweenValue from, to;

  uint size;
  TweenDataType dataType;
  TweenProgressType progressType;
};

typedef Array Tweens;

void initTweens(Tweens &tweens);

// calculate all tweens. Will call free() on from, to if those were defined as pointers
void updateTweens(Tweens &tweens, float deltaTime);

// create a new tween of any kind
void tween(
    Tweens &tweens, float duration, TweenDataType dataType, uint size,
    void *target, TweenValue from, TweenValue to, TweenProgressType progressType);

#define tweenFloat(tweens, duration, target, from, to, progressType) \
  tween(tweens, duration, TweenFloat, 1, (void *)target, from, to, progressType)

#endif