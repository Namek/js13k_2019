#include "game.hpp"

float calcCenterX(float onLanePercent) {
  return BASE_GAME_WIDTH * onLanePercent;
}

// -1 is bottom roadside, laneCount is top roadside
float calcCenterYForLane(int laneIndex) {
  ref level = state.currentLevel;
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