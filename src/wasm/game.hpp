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

#define createCmp(type, entityId) world.createComponent<type>(entityId)

DEF_COMPONENT(Transform)
  float x;           //centerX, pixels on screen
  float y;           //centerY
  float orientation; //rotation in radians
END_COMPONENT

// corresponds to Transform
DEF_COMPONENT(Collider)
  float width;  // pixels on screen
  float height; // pixels on screen
END_COMPONENT

enum VehicleType
{
  NormalCar,
  FastCar,
  TIR
};

// unchangable parameters of a vehicle
struct VehicleStaticParams {
  VehicleType type;

  float maxSpeed;

  // per second, value used in UI so the player can configure speed change up to this value
  float maxSlowdownFactor;

  // per second, value used in UI so the player can configure speed change up to this value
  float maxSpeedupFactor;

  bool playerCanChangeLane;
};

// set by a player, essential part of whole gameplay
struct VehicleConfiguredParams {
  // default is 0, lower than 0 is to the length of `maxSlowdownFactor`, greater than 0 is up to `maxSpeedupFactor`
  float speedChangeFactor;

  // default is 0 which means no change. -1 is lane lower, +1 is upper.
  // Player can change it only if `playerCanChangeLane` is true.
  // int laneChange;
  // TODO maybe it will disappear because there are 2 ways of changing lane: by player, by AI; not sure if those can play well and understandably together.
};

// effect of simulation
struct VehicleDynamicParams {
  float speed;

  int laneIndex_current;
  int laneIndex_target;

  // 0 defines always left, 1 always right. Range 0..1 but values out of range are OK too!
  float onLanePosPercent;

  // 0...1 - when moving from current to target
  float changingLaneProgress;
};

// all starting params for single vehicle
struct VehicleConfig {
  VehicleStaticParams *paramsStatic;
  VehicleDynamicParams paramsDynamicInitial;
};

DEF_COMPONENT(Vehicle)
  VehicleStaticParams *paramsStatic;

  // params set before simulation starts
  VehicleDynamicParams paramsDynamicInitial;

  // params set on currently rendered frame
  VehicleDynamicParams paramsDynamicCurrent;

  // history of frames
  Array /* of VehicleDynamicParams */ paramsDynamicSimulatedFrames;

  VehicleConfiguredParams paramsConfiguredByPlayer;
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

  // TODO simulated frames
END_COMPONENT

#define COMPONENT_TYPE_COUNT __COUNTER__

#define COMPONENT_TYPE_SIZES \
  { SIZE(Transform),         \
    SIZE(Collider),          \
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


enum HorzDir {
  Right = 1, Left = -1
};

struct Lane {
  HorzDir horzDir;
};

struct LevelParams {
  Array /*of VehicleConfig*/ vehicles;
  int maxLaneSpeed;
  Lane *laneDefinitions; //TODO transform it to array?
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
  EcsWorld ecsWorld;
  Level currentLevel;
  Camera camera;

  Phase phase;

  // applies if phase==Rewind
  float rewindProgress;

  // collection of pointers to be free()d up after level finishes
  Array levelGarbage;
};
extern GameState state;

#define GC_LEVEL(pointer) *(int *)state.levelGarbage.createPtr() = (int)(pointer);

#define BOTTOM_UI_HEIGHT 100
#define BASE_GAME_WIDTH 1200
#define BASE_GAME_HEIGHT (675 - BOTTOM_UI_HEIGHT)

void initGame();
void initLevel(int levelIndex);

// game_helpers
float calcCenterX(float onLanePercent);
float calcCenterYForLane(int laneIndex);
Lane &getLaneForVehicle(Level &level, Vehicle &vehicle);

// debug
void debugRect(Transform &t, Collider &c, float z);

#endif
