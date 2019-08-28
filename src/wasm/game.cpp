#include "game.hpp"
#include "engine.hpp"
#include "textures.hpp"
#include "models/models.hpp"

GameState state;
const float FROGGY_JUMPING_TIME = 0.4f;
const float FROGGY_JUMP_AMPLITUDE = 4.5f;

DEF_ENTITY_SYSTEM(SimulateVehicle, A(Vehicle))
  ref v = getCmpE(Vehicle);
  ref vt = getCmpE(Transform);

  // change speed due to player's settings
  ref speedChangeFactor = v.paramsConfiguredByPlayer.speedChangeFactor;
  if (speedChangeFactor != 0) {
    v.paramsDynamicCurrent.speed += speedChangeFactor * deltaTime * deltaTime;
    v.paramsDynamicCurrent.speed = CLAMP(v.paramsDynamicCurrent.speed, 0, v.paramsStatic->maxSpeed);
  }

  // TODO in future, we should simulate some more physics instead of updating horz/vert separately

  // update horizontal position
  ref lane = getLaneForVehicle(state.currentLevel, v);
  float diffX = v.paramsDynamicCurrent.speed * deltaTime * lane.horzDir;
  vt.pos.x += diffX;

  // foresee to decide if needs changing lanes
  // TODO

  // update vertical position (changing lanes)
  if (v.paramsDynamicCurrent.laneIndex_target != v.paramsDynamicCurrent.laneIndex_current) {
    // TODO
    // v.paramsDynamicCurrent.changingLaneProgress
  }

  // TODO save frame
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(CheckCollisions, A(Collider) | A(Transform))
  ref collider = getCmpE(Collider);
  ref transform = getCmpE(Transform);

  // TODO if anyone collides then change game Phase to Rewind

END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(SimulateFroggy, A(Froggy) | A(Transform) | A(Collider))
  ref frog = getCmpE(Froggy);
  ref t = getCmpE(Transform);
  ref c = getCmpE(Collider);

  // cast ray upfront over the road with small amount of timing prediction (wider ray)
  if (frog.state == WaitForJump) {
    if (frog.stateProgress >= 1.0) {
      frog.state = InitJump;
      frog.stateProgress = 0;
    }
    else {
      frog.stateProgress += (deltaTime / frog.froggyThinkingTime);
    }
  }

  // jump if there is no vehicle on sight
  if (frog.state == InitJump) {
    if (!isAnyVehicleOnSight(world, t.pos.x, t.pos.y, 0, -1, c.width * 2)) {
      frog.state = DuringJump;
      frog.stateProgress = 0;
      frog.jumpingFrom[X] = t.pos.x;
      frog.jumpingFrom[Y] = t.pos.y;
      frog.jumpingTo[X] = t.pos.x;
      frog.jumpingTo[Y] = calcCenterYForLane(frog.nextLaneIndex);
      frog.nextLaneIndex -= frog.yDirection;
    }
  }

  // it's currently on the fly
  if (frog.state == DuringJump) {
    frog.stateProgress += (deltaTime / FROGGY_JUMPING_TIME);

    vec3_lerp(t.pos.vec, frog.jumpingFrom, frog.jumpingTo, sin(PI / 2 * frog.stateProgress));
    t.pos.z = sin(PI * frog.stateProgress) * FROGGY_JUMP_AMPLITUDE;

    if (frog.stateProgress >= 1.0f) {
      frog.stateProgress = 0;
      frog.state = WaitForJump;
      t.pos.z = 0;

      if (frog.nextLaneIndex == -2 || frog.nextLaneIndex == state.currentLevel.params.laneCount + 1) {
        frog.state = Done;
      }
    }
  }

  // TODO record a state frame
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(UpdateVehiclePositionForRender, A(Vehicle) | A(Transform))
  ref t = getCmpE(Transform);
  ref v = getCmpE(Vehicle);
  ref paramsCurrent = v.paramsDynamicCurrent;
  ref paramsStatic = v.paramsStatic;

  // t.x is already calculated in simulation or restored from frame
  t.pos.y = calcCenterYForLane(paramsCurrent.laneIndex_current);

  if (paramsCurrent.laneIndex_target != paramsCurrent.laneIndex_current && paramsCurrent.changingLaneProgress >= 0) {
    float nextLaneY = calcCenterYForLane(paramsCurrent.laneIndex_target);
    t.pos.y = LERP(paramsCurrent.changingLaneProgress, t.pos.y, nextLaneY);
  }

  // super basic version without physics:
  // update orientation based on paramsCurrent.changingLaneProgress (amount of 45* degrees rotation)
  // and the Lane.horzDir direction (left or right!)
  ref lane = getLaneForVehicle(state.currentLevel, v);
  mat4_rotateZ(
      t.orientation, mat4_identity(t.orientation),
      toRadian(45) * CLAMP01(paramsCurrent.changingLaneProgress) * (-lane.horzDir));
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(UpdateFroggyForRender, A(Froggy) | A(Transform))
  ref froggy = getCmpE(Froggy);
  ref t = getCmpE(Transform);
  mat4_identity(t.orientation);
  mat4_rotateZ(t.orientation, t.orientation,
               toRadian(froggy.yDirection == Up ? 0 : 180));

  if (froggy.state == DuringJump) {
    mat4_rotateX(t.orientation, t.orientation, -cos(-PI / 2 + PI * (froggy.stateProgress)) * PI_180 * 15.0f);
  }
END_ENTITY_SYSTEM

void initGame() {
  state.levelGarbage.init(sizeof(void *));
  int sizes[] = COMPONENT_TYPE_SIZES;
  initEcsWorld(state.ecsWorld, sizes, COMPONENT_TYPE_COUNT);
  vec3_set(state.camera.pos.vec, 0, 0, 0);
  vec3_set(state.camera.dir.vec, 0, 0, -1);

  initLevel(0);
  state.phase = Simulate; // Intro;
}

bool onEvent(int eventType, int value) {
  if (eventType != EVENT_KEYDOWN) {
    return false;
  }

  if (value == KEY_RIGHT) {
    state.camera.pos.x += tw(0);
  }
  else if (value == KEY_LEFT) {
    state.camera.pos.x -= tw(0);
  }
  else if (value == KEY_UP) {
    state.camera.pos.y -= tw(0);
  }
  else if (value == KEY_DOWN) {
    state.camera.pos.y += tw(0);
  }
  else if (value == 107 /*KP_PLUS*/) {
    state.camera.pos.z -= tw(0);
  }
  else if (value == 109 /*KP_MINUS*/) {
    state.camera.pos.z += tw(0);
  }
  else {
    _l(value);
    return false;
  }

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

  // axis X goes right, axis Y goes down, axis Z goes towards viewer
  mat4_perspective(getProjectionMatrix(), fieldOfView, aspectRatio, 1, 1000.0);
  mat4_multiply(getProjectionMatrix(), getProjectionMatrix(), mat4_fromScaling(mat4Tmp, vec3_set(vec3Tmp, 1, -1, 1)));

  // top left point is 0,0; center is width/2,height/2
  mat4_translate(getViewMatrix(), mat4_identity(getViewMatrix()), vec3_set(vec3Tmp, -canvasWidth / 2, -canvasHeight / 2, -canvasHeight / 2.0));

  // TODO state.camera.dir
  float camZ = -state.camera.pos.z;
  mat4_translate(getViewMatrix(), getViewMatrix(),
                 vec3_set(vec3Tmp, -state.camera.pos.x, -state.camera.pos.y, camZ));
  //TODO camera.dir

  // process logic with ECS World
  EcsWorld &world = state.ecsWorld;
  world.deltaTime = deltaTime * tw(2);

  Phase phase = (Phase)(int)tw(1); // state.phase;

  if (phase == Simulate) {
    SimulateVehicle(world);
    SimulateFroggy(world);
    CheckCollisions(world);
  }
  else if (phase == Rewind) {
  }
  else if (phase == Playing) {
  }
  UpdateVehiclePositionForRender(world);
  UpdateFroggyForRender(world);

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

    ref level = state.currentLevel;
    const int laneCount = level.params.laneCount;
    const float laneHeight = level.params.laneHeight;
    const float lanesGap = level.params.lanesGap;
    const float roadsideHeight = level.params.roadsideHeight;
    const float grassHeight = level.render.grassHeight;
    const float roadHeight = level.render.roadHeight;

    // grass - top
    texQuad(TEXTURE_GRASS,
            0, 0, z, 0, 0,
            w, 0, z, w / grassHeight, 0,
            w, grassHeight, z, w / grassHeight, 1,
            0, grassHeight, z, 0, 1);

    // grass - bottom
    texRect(TEXTURE_GRASS,
            0, h - grassHeight, z, w, grassHeight,
            w / grassHeight, 1.0f);

    float roadY = grassHeight + roadsideHeight;

    // roadsides
    setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
    rect(0, roadY - roadsideHeight, z, w, roadsideHeight);
    rect(0, roadY + roadHeight, z, w, roadsideHeight);

    // black road
    setColorLeftToRight(1, 0.1, 0.1, 0.1, 0, 0, 0);
    rect(0, roadY, z, w, roadHeight);

    // lane gaps - top to bottom
    float laneY = roadY + roadHeight;
    for (int i = 0; i < laneCount - 1; ++i) {
      laneY -= (laneHeight + lanesGap);

      setColor(1, 1, 1, 1);
      rect(0, laneY, z, w, lanesGap);
    }
  }

  // draw frog, we know there's only one
  if (1) {
    auto *frog = world.getFirstEntityByAspect(A(Froggy) | A(Transform) | A(Collider));
    ref froggy = getCmp(Froggy, frog->id);
    ref transform = getCmp(Transform, frog->id);
    ref collider = world.getComponent<Collider>(frog->id);

    renderFrog(transform.pos.vec, transform.orientation);

    // setColor(1, 1, 0, 0);
    // debugRect(transform, collider);
  }

  // draw vehicles
  if (1) {
    FOR_EACH_ENTITY(world, A(Vehicle) | A(Transform))
      ref v = getCmpE(Vehicle);
      ref t = getCmpE(Transform);
      ref c = getCmpE(Collider);
      ref lane = getLaneForVehicle(state.currentLevel, v);

      // debug: car color by type
      float
          x = t.pos.x - c.width / 2,
          y = t.pos.y - c.height / 2,
          frontWidth = 10;

      if (v.paramsStatic->type == FastCar) {
        float offset = frontWidth / c.width;
        texRect(TEXTURE_CHECKERBOARD,
                x, y, z, c.width, c.height,
                2 * (c.width / c.height) - offset, 2, -offset, 0);
      }
      else {
        setColor(1, 0, 0, 1);
        rect(x, y, z, c.width, c.height);
      }

      // debug: front of car
      setColor(1, 1, 1, 0);
      rect(t.pos.x + lane.horzDir * c.width / 2, t.pos.y - c.height / 2, z, frontWidth, c.height);

      if (v.paramsStatic->type == NormalCar) {
        // TODO pick a 3d model
      }

      if (state.phase == Playing) {
        // TODO special effects
      }
    END_FOR_EACH
  }

  endFrame();
}