#include "game.hpp"

RecordedFrame *getFrame(int frameIndex) {
  return (RecordedFrame *)state.recordedFrames.getPointer(frameIndex);
}

void recordFrame(float deltaTime) {
  ref world = state.ecsWorld;

  FOR_EACH_ENTITY(world, A(Vehicle) | A(Transform))
    ref v = getCmpE(Vehicle);
    ref vt = getCmpE(Transform);

    auto newFrame = (VehicleTimeFrame *)v.paramsDynamicSimulatedFrames.createPtrAt(state.currentFrame);
    newFrame->transform = vt;
    newFrame->params = v.paramsDynamicCurrent;
  END_FOR_EACH

  FOR_EACH_ENTITY(world, A(Froggy) | A(Transform))
    ref frog = getCmpE(Froggy);
    ref t = getCmpE(Transform);
    ref fstate = frog.state;

    auto newFrame = (FroggyTimeFrame *)frog.simulatedFrames.createPtrAt(state.currentFrame);
    newFrame->transform = t;
    newFrame->state = fstate;
  END_FOR_EACH

  auto frame = (RecordedFrame *)state.recordedFrames.createPtr();
  frame->deltaTime = deltaTime;

  float totalTime = 0;
  if (state.currentFrame > 0) {
    auto prevFrame = getFrame(state.currentFrame - 1);
    totalTime = prevFrame->totalTime;
  }
  frame->totalTime = totalTime + deltaTime;

  state.currentFrame += 1;
}

RecordedFrame *loadFrame(int frameIndex) {
  if (frameIndex >= state.recordedFrames.size)
    return NULL;

  state.currentFrame = frameIndex;
  _lstr("load/frame", state.currentFrame);

  ref world = state.ecsWorld;

  FOR_EACH_ENTITY(world, A(Vehicle) | A(Transform))
    ref v = getCmpE(Vehicle);
    ref vt = getCmpE(Transform);

    auto frame = (VehicleTimeFrame *)v.paramsDynamicSimulatedFrames.getPointer(state.currentFrame);
    vt.pos = frame->transform.pos;
    memcpy(vt.orientation, frame->transform.orientation, MAT_SIZE_4 * sizeof(float));
    v.paramsDynamicCurrent = frame->params;
  END_FOR_EACH

  FOR_EACH_ENTITY(world, A(Froggy) | A(Transform))
    ref frog = getCmpE(Froggy);
    ref ft = getCmpE(Transform);
    ref fstate = frog.state;

    auto frame = (FroggyTimeFrame *)frog.simulatedFrames.getPointer(state.currentFrame);
    ft.pos = frame->transform.pos;
    memcpy(ft.orientation, frame->transform.orientation, MAT_SIZE_4 * sizeof(float));
    frog.state = frame->state;
    _lfstr("load/frog.pos.y", frame->transform.pos.y);
  END_FOR_EACH

  return getFrame(frameIndex);
}

void animateRewind(float deltaTime, float debugSpeedFactor) {
  float dtLeft = (state.rewindCurrentFrameDtLeft + deltaTime) * REWIND_TIME_FACTOR * debugSpeedFactor;

  uint frameIndex = state.currentFrame;
  while (dtLeft >= 0 && frameIndex > 0) {
    auto frame = (RecordedFrame *)state.recordedFrames.getPtr(frameIndex--);
    dtLeft -= frame->deltaTime;
  }
  state.rewindCurrentFrameDtLeft = -dtLeft;
  auto frame = loadFrame(frameIndex);

  if (frameIndex == 0) {
    goToPhase(Playing);
  }
  else {
    float leftTime = frame->totalTime / (REWIND_TIME_FACTOR * debugSpeedFactor);

    if (leftTime <= REWIND_ANIM_SHADER_EFFECT_TURN_OFF_DURATION) {
      // turn the shader down
      state.shaderRewind = CLAMP01(leftTime / REWIND_ANIM_SHADER_EFFECT_TURN_OFF_DURATION);
    }
  }
}