#include "game.hpp"
#include "engine.hpp"
#include "textures.hpp"
#include "models/models.hpp"

#define BOTTOM_UI_HEIGHT 100
#define BASE_GAME_WIDTH 1200
#define BASE_GAME_HEIGHT (675 - BOTTOM_UI_HEIGHT)
GameState state;

// VehicleParams allVehicleParams[]= {

// };


float tweakValues[10];
void setTweakValue(int index, float value) {
  tweakValues[index] = value;
}
#define tw(index) tweakValues[index]

DEF_ENTITY_SYSTEM(UpdateVehiclePositions, A(Vehicle) | A(Transform))
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(CheckCollisions, A(Collidable) | A(Transform))
  ref collider = getCmpE(Collidable);
  ref transform = getCmpE(Transform);

  // TODO if anyone collides then change game Phase to Rewind

END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(UpdateFroggy, A(Froggy) | A(Transform))
  ref frog = getCmpE(Froggy);
END_ENTITY_SYSTEM

void initLevel(int levelIndex) {
  EcsWorld &world = state.ecsWorld;
  LevelParams level;

  level.lanesGap = 4;
  level.roadsideHeight = 40;
  level.laneHeight = 70;

  switch (levelIndex) {
  default:
  case 0:
    level.froggyXPosition = 0.2;
    level.froggyThinkingTime = 1.0f;
    level.maxLaneSpeed = 80;
    level.laneCount = 4;

    // Entity &e = world.newEntity();
    // e.add(level);
  }

  state.currentLevel.params = level;
  state.currentLevel.render.roadHeight = level.laneCount * level.laneHeight + ((level.laneCount - 1) * level.lanesGap);
  state.currentLevel.render.grassHeight = (BASE_GAME_HEIGHT - state.currentLevel.render.roadHeight - 2 * level.roadsideHeight) / 2;

  // Frog
  {
    ref frog = world.newEntity();
    ref froggy = world.createComponent<Froggy>(frog.id);
    froggy.froggyThinkingTime = state.currentLevel.params.froggyThinkingTime;
    froggy.state = WaitForJump;
    froggy.stateProgress = 0;

    ref transform0 = world.createComponent<Transform>(frog.id);
    ref transform = world.createComponent<Transform>(frog.id);
    transform.orientation = 0;
    transform.x = BASE_GAME_WIDTH * state.currentLevel.params.froggyXPosition;
    transform.y = BASE_GAME_HEIGHT - state.currentLevel.render.grassHeight / 2;

    ref collider = world.createComponent<Collidable>(frog.id);
    collider.width = 40;
    collider.height = 50;
  }
}
const char *bits = "bits";
void initGame() {
  int componentTypeSizes[] = COMPONENT_TYPE_SIZES;
  initEcsWorld(state.ecsWorld, componentTypeSizes, COMPONENT_TYPE_COUNT);
  vec3_set(state.camera.pos, 0, 0, 0);
  vec3_set(state.camera.dir, 0, 0, -1);

  state.phase = Playing; // Intro;

  initLevel(0);
}

bool onEvent(int eventType, int value) {
  if (eventType != EVENT_KEYDOWN) {
    return false;
  }

  _l(tw(0));

  if (value == KEY_RIGHT) {
    state.camera.pos[X] += tw(0);
  }
  else if (value == KEY_LEFT) {
    state.camera.pos[X] -= tw(0);
  }
  else if (value == KEY_UP) {
    state.camera.pos[Y] += tw(0);
  }
  else if (value == KEY_DOWN) {
    state.camera.pos[Y] -= tw(0);
  }
  else
    return false;

  return true;
}



// returns numbers:
//  - number of vertices (same as number of colors)
//  - number of indices
void render(float deltaTime) {
  beginFrame();

  // TODO apply scale between BASE_GAME_WIDTH and `w` etc. - in case of canvas resize
  float canvasWidth = getCanvasWidth();
  float canvasHeight = getCanvasHeight();
  float h = canvasHeight - BOTTOM_UI_HEIGHT;
  float w = canvasWidth;
  float w2 = w / 2.0;
  float h2 = h / 2.0;

  const float aspectRatio = canvasWidth / canvasHeight;
  const float fieldOfView = 90.0 * PI_180;

  const float zNear = 0;

  //setProjectionMatrix(fieldOfView, aspectRatio, 1.0, 2000.0);
  mat4_perspective(getProjectionMatrix(), fieldOfView, aspectRatio, 1, 1000.0);
  mat4_multiply(getProjectionMatrix(), getProjectionMatrix(), mat4_fromScaling(mat4Tmp, vec3_set(vec3Tmp, 1, -1, 1)));
  // setProjectionMatrix(mat4_frustum(mat4Tmp, 0, canvasWidth, canvasHeight, 0, -zNear, 1000.0));

  // top left point is 0,0; center is width/2,height/2
  mat4_set(getViewMatrix(),
           1.0, 0.0, 0.0, 0.0,
           0.0, 1.0, 0.0, 0.0,
           0.0, 0.0, 1.0, 0.0,
          //  -canvasWidth / 2, -canvasHeight / 2, -canvasHeight / 2.0, 1.0);
           0,0,0, 1.0);

  triangle(50, h / 2, zNear, 100, h / 2, zNear, 100, h, zNear);

  // TODO state.camera.dir
  float camZ = -tw(1);// -state.camera.pos[Z];
  mat4_translate(getViewMatrix(), getViewMatrix(),
                 vec3_set(vec3Tmp, -state.camera.pos[X], -state.camera.pos[Y], camZ));
  //TODO camera.dir

  // process logic with ECS World
  EcsWorld &world = state.ecsWorld;

  world.deltaTime = deltaTime;
  UpdateVehiclePositions(world);
  UpdateFroggy(world);
  CheckCollisions(world);

  const float z = zNear;
  if (1) {

    // layout here is:
    // - grass
    // - roadside
    // - road:
    //   - (lane, gap)...
    //   - lane
    // - roadside
    // - grass

    auto &level = state.currentLevel;
    const int laneCount = level.params.laneCount;
    const float laneHeight = level.params.laneHeight;
    const float lanesGap = level.params.lanesGap;
    const float roadsideHeight = level.params.roadsideHeight;
    const float grassHeight = level.render.grassHeight;
    const float roadHeight = level.render.roadHeight;

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

    float z2 = z - 40;
    triangle(100, 100, z, 100, 300, z, 400, 300, z);
    setColor(1, 1, 0, 0);
    triangle(100, 100, z, 100, 300, z, 100, 100, z2);
    setColor(1, 1, 0, 1);
    triangle(100, 100, z2, 400, 300, z2, 400, 100, z2);

    // roadsides
    setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
    rect(0, roadY - roadsideHeight, z, w, roadsideHeight);
    rect(0, roadY + roadHeight, z, w, roadsideHeight);

    // // black road
    setColorLeftToRight(1, 0.1, 0.1, 0.1, 0, 0, 0);
    rect(0, roadY, z, w, roadHeight);

    // lane gaps - top to down
    float laneY = roadY + roadHeight;
    for (int i = 0; i < laneCount - 1; ++i) {
      laneY -= (laneHeight + lanesGap);

      setColor(1, 1, 1, 1);
      rect(0, laneY, z, w, lanesGap);
    }
  }

  // draw frog, we know there's only one
  if (1) {
    auto *frog = world.getFirstEntityByAspect(A(Froggy));

    ref froggy = getCmp(Froggy, frog->id);
    ref transform = getCmp(Transform, frog->id);
    ref collider = world.getComponent<Collidable>(frog->id);
    float
        x = transform.x - collider.width / 2,
        y = transform.y - collider.height / 2;

    setColor(1, 1, 0, 0.4f);

    auto matFrog = mat4_identity(pushModelMatrix());
    float frogScale = 1;
    mat4_translate(matFrog, matFrog, vec3_set(vec3Tmp, 10, 0,0));
    mat4_scale(matFrog, matFrog, vec3_set(vec3Tmp, frogScale, frogScale, frogScale));
    // mat4_rotateX(matFrog, matFrog, toRadian(-90));

    setColor(1, 1, 0, 0);
    renderModel3d(getModel_frog());
    setModelMatrix(matFrog);
    popModelMatrix();

    setColor(1, 1, 0, 0);
    // rect(x, y, z, collider.width, collider.height);
  }
  // rect(100, 100, z, 300, 150);

  endFrame();
}