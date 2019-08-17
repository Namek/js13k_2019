#include "game.hpp"
#include "textures.hpp"
#include "ecs.hpp"

// let's estimate size of history of frames
// 100 entities * 16 bytes * 60 frames per second * 15 seconds / 1024 bytes = 1406 kB

enum GameState
{
  Intro,
  LevelPresentation, //showing level number, some text
  Playing,
  Simulate,
  Rewind,
};

enum EntityType
{
  Car,
  TIR,
  Frog,
  // Hedgehog,
  // Cat
};

DEF_COMPONENT(Transform) {
  EntityType type;
  float x;
  float y;
  float orientation;
};

DEF_COMPONENT(Movement) {
  float velocityX;
  float velocityY;
};

DEF_COMPONENT(Vehicle) {
  int laneIndex;
};

DEF_COMPONENT(Level) {
  int laneCount;
};

#define COMPONENT_TYPES_COUNT __COUNTER__


class PositioningSystem : AAEntitySystem<Transform, Movement> {
  void processEntity(Entity& entity) {
    Transform& t = getCmp<Transform, 1>();
  }
};


WASM_EXPORT
void asdomg() {
  AAWorld<Components<Transform,Level>, Systems<PositioningSystem>, 1000> world;

// TODO consider returning int (index or id??)
  Entity& e = world.newEntity();

  world.process();

  // Transform& t = world.components.create<Transform>();

// TODO from this point  <-----------------------------------------


  // AAComponentMapper<Transform> mTransform& = world.mapper<Transform>();

  // Transform& t = mTransform[e].get();
}



// EcsWorld<Components<Transform, Level, Vehicle>, 1000> world;
typedef  Components<Transform, Level, Vehicle> ComponentTypes;
auto ct = ComponentTypes();

EcsWorld<1000, 50, ComponentTypes, ct > world;

void initLevel(int levelIndex) {
  Level level;

  switch (levelIndex) {
  default:
  case 0:
    level.laneCount = 4;

    // Entity &e = world.newEntity();
    // e.add(level);
  }
}









GameState gameState = Playing; // Intro;

// returns numbers:
//  - number of vertices (same as number of colors)
//  - number of indices
void render(float deltaTime) {
  beginFrame();

  Entity &e = world.newEntity();
  Level level;
  e.add(level);

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
  int laneCount = 4;
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

  // entities (animals, cars, people)

  endFrame();
}