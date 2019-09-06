#include "common.hpp"
#include "math/common.h"
#include "tween.hpp"

void initTweens(Tweens &tweens) {
  tweens.init(sizeof(Tween));
}

void updateTweens(Tweens &tweens, float deltaTime) {
  for (uint i = 0, n = tweens.size; i < n; ++i) {
    Ref t = *(Tween *)tweens.getPointer(i);

    if (!t.active)
      continue;

    t.progress = t.progress + deltaTime / t.duration;

    float alpha = t.progress;

    if (t.progressType == Linear) {
    }
    else if (t.progressType == Sin) {
      alpha = sin(alpha);
    }
    else if (t.progressType == Cos) {
      alpha = cos(alpha);
    }
    else {
#ifndef PRODUCTION
      _abort("not implemented tween progress type");
#endif
    }

    if (t.dataType == TweenFloat) {
      *(float *)t.target = LERP(CLAMP01(t.progress), t.from.floatVal, t.to.floatVal);
    }
    else {
#ifndef PRODUCTION
      _abort("not implemented tween data type");
#endif
    }

    if (t.progress >= 1.0f) {
      t.active = false;
      tweens.remove(i);
    }
  }
}

void tween(
    Tweens &tweens, float duration, TweenDataType dataType, uint size,
    void *target, TweenValue from, TweenValue to, TweenProgressType progressType) {

  Ref t = *(Tween *)tweens.createPtr();
  t.active = true;
  t.duration = duration;
  t.dataType = dataType;
  t.size = size;
  t.target = target;
  t.from = from;
  t.to = to;
  t.progressType = progressType;
}