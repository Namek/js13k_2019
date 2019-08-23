#ifndef GAME__H
#define GAME__H

#include "common.hpp"
#include "ecs.hpp"
#include "math/common.h"

extern "C" {
WASM_EXPORT void initGame();
}

// to be used in Entity System, uses local `entity` variable
#define getCmpE(type) (type &)world.getComponent<type>(entity.id)

// to be used everywhere else, requires specifying entity id by hand
#define getCmp(type, entityId) world.getComponent<type>(entityId)

#define ref auto &

DEF_COMPONENT(Transform)
  float x;           //centerX, pixels on screen
  float y;           //centerY
  float orientation; //rotation in radians
END_COMPONENT

// corresponds to Transform
DEF_COMPONENT(Collidable)
  float width;  // pixels on screen
  float height; // pixels on screen
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

  // state == WaitForJump: delay between jumps
  float froggyThinkingTime;

  // how long it jumps every time, sized with pixels on screen
  float leapLength;

  // 0..1 describes both state / amount of waiting time and animation
  float stateProgress;
END_COMPONENT

#define COMPONENT_TYPE_COUNT __COUNTER__

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
  float laneHeight;
  float lanesGap;
  float roadsideHeight;
  float froggyThinkingTime; //seconds
  float froggyXPosition;    //0..1 - percent of screen
};

// based on laneHeight, laneCount
struct LevelRenderCache {
  float roadHeight;
  float grassHeight;
};

struct Level {
  int index;
  LevelParams params;
  LevelRenderCache render;
};

struct Camera {
  float *pos; //vec3
  float *dir; //vec3
};

struct GameState {
  Phase phase;
  EcsWorld ecsWorld;
  Level currentLevel;
  Camera camera;
};

#endif
