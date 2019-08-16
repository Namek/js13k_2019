#include "game.hpp"
#include "textures.hpp"

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

struct Entity {
  int componentBitSet;
};

struct Component {};

#define DEF_COMPONENT(name) \
  struct name : Component { \
    static const int I = __COUNTER__;

#define DEF_END \
  }             \
  ;

DEF_COMPONENT(Transform)
  EntityType type;
  float x;
  float y;
  float orientation;
DEF_END

DEF_COMPONENT(Vehicle)
  int laneIndex;
DEF_END

template <class T>
class Array {
  private:
  int size;

  public:
  T *data;

  Array(int size) {
    this->data = new T[size];
    this->size = size;
  }

  ~Array() {
    delete[] data;
    size = 0;
  }

  void set(int index, T *el) {
    data[index] = el;
  }

  T &get(int index) {
    return data[index];
  }
};

class EntitySystem {
  public:
  int componentBitSet;
  void processEntity(Entity &entity);
};

class EcsWorld {
  private:
  Entity *_entities;
  Component *_components;
  EntitySystem *_systems;
  int _maxEntityCount;
  int _entitySystemCount;
  int _lastEntityIndex;

  public:
  EcsWorld(int maxEntityCount, int componentTypeCount, EntitySystem *systems) {
    _maxEntityCount = maxEntityCount;
    _entities = new Entity[maxEntityCount];
    _components = new Component[componentTypeCount * maxEntityCount];
    _systems = systems;
    _entitySystemCount = 0;
    _lastEntityIndex = -1;
  }

  Entity &getEntity(int index) {
    return _entities[index];
  }

  // createEntity

  void process() {
    for (int is = 0; is < _entitySystemCount; ++is) {
      EntitySystem &system = _systems[is];
      int systemAspect = system.componentBitSet;

      for (int ie = ie; ie < _lastEntityIndex; ++ie) {
        Entity &entity = _entities[ie];
        if ((entity.componentBitSet & systemAspect) != 0) {
          system.processEntity(entity);
        }
      }
    }
  }
};

struct LevelParams {
  int laneCount;
};

LevelParams initLevel(int levelIndex) {
  LevelParams p;
  switch (levelIndex) {
  default:
  case 0:
    p.laneCount = 4;
  }
  return p;
}

GameState gameState = Playing; // Intro;

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
