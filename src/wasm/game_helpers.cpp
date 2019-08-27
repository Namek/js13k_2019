#include "game.hpp"

float calcCenterX(float onLanePercent) {
  return BASE_GAME_WIDTH * onLanePercent;
}

// * [0..laneCount-1] are lanes
// * -1 is bottom roadside
// * laneCount is top roadside
// * laneCount+1 and -2 are grasses
float calcCenterYForLane(int laneIndex) {
  ref level = state.currentLevel;

  if (laneIndex == -2)
    return BASE_GAME_HEIGHT - state.currentLevel.render.grassHeight / 2;
  else if (laneIndex == state.currentLevel.params.laneCount + 1)
    return state.currentLevel.render.grassHeight / 2;

  float y = BASE_GAME_HEIGHT - state.currentLevel.render.grassHeight;

  if (laneIndex >= 0)
    y -= level.params.roadsideHeight;
  else {
    y -= level.params.roadsideHeight / 2;
    return y;
  }

  // go to center of bottom lane
  y -= level.params.laneHeight / 2;

  // if we want other lane then jump every whole lane (including the gap)
  float halfLane = level.params.laneHeight / 2 + level.params.lanesGap / 2;
  for (int i = 0; i < laneIndex; ++i) {
    y -= halfLane*2;
  }

  if (laneIndex == level.params.laneCount) {
    // we jumped too far
    y += halfLane;

    // from edge of the roadside to it's center
    y -= level.params.roadsideHeight / 2;
  }

  return y;
}

Lane &getLaneForVehicle(Level &level, Vehicle &vehicle) {
  return state.currentLevel.params.laneDefinitions[vehicle.paramsDynamicCurrent.laneIndex_current];
}

bool isAnyVehicleOnSight(EcsWorld& world, float x, float y, float xDir, float yDir, float rayWidth) {

  FOR_EACH_ENTITY(world, A(Vehicle) | A(Transform) | A(Collider))
    ref v = getCmpE(Vehicle);
    ref t = getCmpE(Transform);
    ref c = getCmpE(Collider);

    // TODO AABB
  END_FOR_EACH


  return false;
}