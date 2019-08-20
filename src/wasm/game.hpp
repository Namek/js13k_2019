#ifndef GAME__H
#define GAME__H

#include "ecs.hpp"
#include "common.hpp"

extern "C" {
WASM_EXPORT void initGame();
}

// to be used in Entity System
#define getCmp(type) world.getComponent<type>(entity.id)

DEF_COMPONENT(Transform)
  float x;           //centerX
  float y;           //centerY
  float orientation; //rotation in radians
END_COMPONENT

// corresponds to Transform
DEF_COMPONENT(Collidable)
  float width;
  float height;
END_COMPONENT

struct VehicleParams {
  int modelType;

  float maxSpeed;

  // how much of meters can it slow down per second
  float slowdownFactor;

  // how much of meters can it speed up per second
  float speedupFactor;
};

DEF_COMPONENT(Vehicle)
  VehicleParams params;

  int defaultParamsIndex;

  int targetLaneIndex;
  int currentLaneIndex;

  float changingLaneProgress; // 0...1 - when moving from current to target
END_COMPONENT

enum FrogState
{
  Jump,
  WaitForJump
};
DEF_COMPONENT(Froggy)
  FrogState state;
  float froggyThinkingTime;

  // 0..1 describes both state / amount of waiting time and animation
  float stateProgress;
END_COMPONENT

#define COMPONENT_TYPES_COUNT __COUNTER__

#define COMPONENT_TYPE_SIZES \
  { SIZE(Transform),         \
    SIZE(Collidable),        \
    SIZE(Vehicle),           \
    SIZE(Froggy) }

enum Phase
{
  Intro,
  LevelPresentation, //showing level number, some text
  Playing,
  Simulate,
  Rewind,
};

struct LevelParams {
  int maxLaneSpeed;
  int laneCount;
  float froggyThinkingTime; //seconds
  float froggyXPosition;    //0..1 - percent of screen
};

struct Level {
  int index;
  LevelParams params;
};

struct GameState {
  Phase phase;
  EcsWorld ecsWorld;
  Level currentLevel;
};

#endif