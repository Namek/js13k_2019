#include "game.hpp"
#include "engine.hpp"
#include "textures.hpp"

GameState state;

// VehicleParams allVehicleParams[]= {

// };

DEF_ENTITY_SYSTEM(UpdateVehiclePositions, A(Vehicle) | A(Transform))
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(CheckCollisions, A(Collidable) | A(Transform))
  auto collider = world.getComponent<Collidable>(entity.id);
  auto transform = world.getComponent<Transform>(entity.id);

  // TODO if anyone collides then change game Phase to Rewind

END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(UpdateFroggy, A(Froggy) | A(Transform))
  auto frog = getCmp(Froggy);
END_ENTITY_SYSTEM

void initLevel(int levelIndex) {
  EcsWorld &world = state.ecsWorld;
  LevelParams level;

  switch (levelIndex) {
  default:
  case 0:
    level.froggyXPosition = 0.3;
    level.froggyThinkingTime = 1.0f;
    level.maxLaneSpeed = 80;
    level.laneCount = 4;

    // Entity &e = world.newEntity();
    // e.add(level);
  }

  state.currentLevel.params = level;

  {
    auto frog = world.newEntity();
    auto froggy = world.createComponent<Froggy>(frog.id);
    froggy.froggyThinkingTime = level.froggyThinkingTime;
    froggy.stateProgress = 0;
    froggy.state = WaitForJump;

    auto transform = world.createComponent<Transform>(frog.id);
    transform.orientation = 0;
    transform.x = level.froggyXPosition;
  }
}

void initGame() {
  int componentTypeSizes[] = COMPONENT_TYPE_SIZES;
  initEcsWorld(state.ecsWorld, componentTypeSizes);
  // state.phase = Playing; // Intro;

  // initLevel(0);
}

// returns numbers:
//  - number of vertices (same as number of colors)
//  - number of indices
void render(float deltaTime) {
  beginFrame();

  float canvasWidth = getCanvasWidth();
  float canvasHeight = getCanvasHeight();
  float bottomUiHeight = 100;
  float h = canvasHeight - bottomUiHeight;
  float w = canvasWidth;
  float w2 = w / 2.0;
  float h2 = h / 2.0;

  const float aspectRatio = canvasWidth / canvasHeight;
  const float fieldOfViewInDegrees = 90.0 * PI_180;
  setProjectionMatrix(fieldOfViewInDegrees, aspectRatio, 1.0, 2000.0);

  // process logic with ECS World
  EcsWorld &world = state.ecsWorld;

  world.deltaTime = deltaTime;
  UpdateVehiclePositions(world);
  UpdateFroggy(world);
  CheckCollisions(world);

  // bottom left point is 0,0; center is width/2,height/2
  const float cameraZ = canvasHeight / 2.0;
  setViewMatrix(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      -canvasWidth / 2, -canvasHeight / 2 + bottomUiHeight, -cameraZ, 1.0);

  // background
  const float z = 0;

  // layout here is:
  // - grass
  // - roadside
  // - road:
  //   - (lane, gap)...
  //   - lane
  // - roadside
  // - grass

  float laneHeight = 70;
  const int laneCount = state.currentLevel.params.laneCount;
  float lanesGap = 4;
  float roadsideHeight = 40;
  float roadHeight = laneCount * laneHeight + ((laneCount - 1) * lanesGap);

  float grassHeight = (h - roadHeight - 2 * roadsideHeight) / 2;

  // grass - top
  setColors4(1,
             0, 1, 0.04,
             0, 1, 0.04,
             0, 0.7, 0.04,
             0, 0.7, 0.04);
  quad(
      0, 0, z,
      0, grassHeight, z,
      w, grassHeight, z,
      w, 0, z);

  // grass - bottom
  setColors4(1,
             0, 1, 0.04,
             0, 0.7, 0.04,
             0, 0.7, 0.04,
             0, 1, 0.04);
  quad(
      0, h, z,
      w, h, z,
      w, h - grassHeight, z,
      0, h - grassHeight, z);

  float roadY = grassHeight + roadsideHeight;

  // roadsides
  setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
  rect(0, roadY - roadsideHeight, z, w, roadsideHeight);
  rect(0, roadY + roadHeight, z, w, roadsideHeight);

  // black road
  setColorLeftToRight(1, 0.1, 0.1, 0.1, 0, 0, 0);
  rect(0, roadY, z, w, roadHeight);

  // lane gaps - top to down
  float laneY = roadY + roadHeight;
  for (int i = 0; i < laneCount - 1; ++i) {
    laneY -= (laneHeight + lanesGap);

    setColor(1, 1, 1, 1);
    rect(0, laneY, z, w, lanesGap);
  }

  // draw frog, we know there's only one
  {
    auto frog = world.getFirstEntityByAspect(A(Froggy));
    auto froggy = world.getComponent<Froggy>(frog->id);
    auto transform = world.getComponent<Transform>(frog->id);
  }

  endFrame();
}