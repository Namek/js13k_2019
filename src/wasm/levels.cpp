#include "game.hpp"

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
    levelParams.froggyXPosition = 0.2;
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
    ref vs0 = vehiclesStatic[0];
    vs0.type = NormalCar;
    vs0.maxSlowdownFactor = 0.4;
    vs0.maxSpeedupFactor = 0.4;
    vs0.playerCanChangeLane = true;
    vs0.maxSpeed = 0.01;
    ref vs1 = vehiclesStatic[1];
    vs1.type = FastCar;
    vs1.maxSlowdownFactor = 0.7;
    vs1.maxSpeedupFactor = 0.7;
    vs1.playerCanChangeLane = true;
    vs1.maxSpeed = vs0.maxSpeed * 3;

    // all vehicles on level
    levelParams.vehicles.init(sizeof(VehicleConfig));

    auto vc0 = (VehicleConfig *)levelParams.vehicles.createPtr();
    vc0->paramsStatic = vehiclesStatic + 0;
    vc0->paramsDynamicInitial.changingLaneProgress = 0;
    uint laneIndex = 0;
    vc0->paramsDynamicInitial.laneIndex_target = laneIndex;
    vc0->paramsDynamicInitial.laneIndex_current = laneIndex;
    vc0->paramsDynamicInitial.onLanePosPercent = 0.4;
    float maxSpeedFactor = 0.8;
    vc0->paramsDynamicInitial.speed = vc0->paramsStatic->maxSpeed * maxSpeedFactor;
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

      vc.height = 70;
      vc.width = 120;

      // transform will be set up automatically by system
    }
  }
}
