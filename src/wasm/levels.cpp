#include "game.hpp"

VehicleStaticParams &setupVehicleStaticParams(
    VehicleStaticParams &vs,
    VehicleType type,
    float maxSlowdownFactor,
    float maxSpeedupFactor,
    bool playerCanChangeLane,
    float maxSpeed) {
  vs.type = type;
  vs.maxSlowdownFactor = maxSlowdownFactor;
  vs.maxSpeedupFactor = maxSpeedupFactor;
  vs.playerCanChangeLane = playerCanChangeLane;
  vs.maxSpeed = maxSpeed;

  return vs;
}

VehicleConfig *newVehicleConfig(LevelParams &levelParams, VehicleStaticParams *paramsStatic, int laneIndex, float maxSpeedPercent, float onLanePosPercent) {
  auto vc = (VehicleConfig *)levelParams.vehicles.createPtr();
  vc->paramsStatic = paramsStatic;
  vc->paramsDynamicInitial.changingLaneProgress = 0;
  vc->paramsDynamicInitial.laneIndex_target = laneIndex;
  vc->paramsDynamicInitial.laneIndex_current = laneIndex;
  vc->paramsDynamicInitial.onLanePosPercent = onLanePosPercent;
  vc->paramsDynamicInitial.speed = vc->paramsStatic->maxSpeed * maxSpeedPercent;
  return vc;
}

void initLevel(int levelIndex) {
  // cleanup after previous level
  for (int i = 0; i < state.levelGarbage.size; ++i) {
    free(*(void **)state.levelGarbage.getPtr(i));
  }
  state.levelGarbage.size = 0;

  // init new level
  EcsWorld &world = state.ecsWorld;
  LevelParams levelParams;

  levelParams.lanesGap = 4;
  levelParams.roadsideHeight = 40;
  levelParams.laneHeight = 70;

  VehicleStaticParams *vehiclesStatic;

  switch (levelIndex) {
  default:
  case 0:
    levelParams.froggyXPosition = 0.8;
    levelParams.froggyThinkingTime = 1.0f;
    levelParams.maxLaneSpeed = 80;

    // lanes
    levelParams.laneCount = 4;
    GC_LEVEL(levelParams.laneDefinitions = Allocate(Lane, levelParams.laneCount));
    levelParams.laneDefinitions[0].horzDir = Right;
    levelParams.laneDefinitions[1].horzDir = Right;
    levelParams.laneDefinitions[2].horzDir = Left;
    levelParams.laneDefinitions[3].horzDir = Left;

    // static params
    GC_LEVEL(vehiclesStatic = Allocate(VehicleStaticParams, 2));

    ref normalCar = setupVehicleStaticParams(vehiclesStatic[0], NormalCar, 0.4, 0.4, true, 0.01);
    ref fastCar =  setupVehicleStaticParams(vehiclesStatic[1], FastCar, 0.7, 0.7, true, normalCar.maxSpeed * 3);

    // all vehicles on level
    levelParams.vehicles.init(sizeof(VehicleConfig));
    newVehicleConfig(levelParams, vehiclesStatic + 0, 0, 0.8, 0.4);
    newVehicleConfig(levelParams, vehiclesStatic + 0, 0, 0.8, 0.2);
    newVehicleConfig(levelParams, vehiclesStatic + 1, 1, 0.8, 0.7);
    newVehicleConfig(levelParams, vehiclesStatic + 0, 2, 0.8, 0.1);
    newVehicleConfig(levelParams, vehiclesStatic + 1, 3, 1, 0.4);
  }

  state.currentLevel.params = levelParams;
  state.currentLevel.render.roadHeight = levelParams.laneCount * levelParams.laneHeight + ((levelParams.laneCount - 1) * levelParams.lanesGap);
  state.currentLevel.render.grassHeight = (BASE_GAME_HEIGHT - state.currentLevel.render.roadHeight - 2 * levelParams.roadsideHeight) / 2;

  // Frog
  {
    ref frog = world.newEntity();
    ref froggy = world.createComponent<Froggy>(frog.id);
    froggy.froggyThinkingTime = state.currentLevel.params.froggyThinkingTime;
    froggy.state = WaitForJump;
    froggy.stateProgress = 0;

    ref transform = world.createComponent<Transform>(frog.id);
    transform.orientation = 0;
    transform.x = BASE_GAME_WIDTH * state.currentLevel.params.froggyXPosition;
    transform.y = BASE_GAME_HEIGHT - state.currentLevel.render.grassHeight / 2;

    ref collider = world.createComponent<Collider>(frog.id);
    collider.width = 30;
    collider.height = 35;
  }

  // Vehicles
  {
    ref vehicles = state.currentLevel.params.vehicles;

    for (int vi = 0; vi < vehicles.size; ++vi) {
      auto vehicleConfig = (VehicleConfig *)vehicles.getPointer(vi);

      ref e = world.newEntity();
      ref v = createCmp(Vehicle, e.id);
      ref vt = createCmp(Transform, e.id);
      ref vc = createCmp(Collider, e.id);

      v.paramsDynamicInitial = vehicleConfig->paramsDynamicInitial;
      v.paramsDynamicCurrent = vehicleConfig->paramsDynamicInitial;
      v.paramsStatic = vehicleConfig->paramsStatic;
      v.paramsDynamicSimulatedFrames.init(sizeof(VehicleDynamicParams));
      v.paramsConfiguredByPlayer.speedChangeFactor = 0;
      // v.paramsConfiguredByPlayer.laneChange = 0;

      vc.height = state.currentLevel.params.laneHeight * 0.85f;
      vc.width = 120;

      // transform will be set up automatically by system
    }
  }
}
