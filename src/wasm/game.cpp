#include "game.hpp"
#include "models/models.hpp"
#include "textures.hpp"

GameState state;

DEF_ENTITY_SYSTEM(SimulateVehicle, A(Vehicle))
  ref v = getCmpE(Vehicle);
  ref vt = getCmpE(Transform);

  // change speed due to player's settings
  // ref speedChangeFactor = v.paramsConfiguredByPlayer.speedChangeFactor;
  // if (speedChangeFactor != 0) {
  //   v.paramsDynamicCurrent.speed += speedChangeFactor * deltaTime * deltaTime;
  //   v.paramsDynamicCurrent.speed = CLAMP(v.paramsDynamicCurrent.speed, 0, v.paramsStatic->maxSpeed);
  // }

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
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(SimulateFroggy, A(Froggy) | A(Transform) | A(Collider))
  ref frog = getCmpE(Froggy);
  ref t = getCmpE(Transform);
  ref c = getCmpE(Collider);
  ref fstate = frog.state;

  // cast ray upfront over the road with small amount of timing prediction (wider ray)
  if (fstate.phase == WaitForJump) {
    if (fstate.phaseProgress >= 1.0) {
      fstate.phase = InitJump;
      fstate.phaseProgress = 0;
    }
    else {
      fstate.phaseProgress += (deltaTime / state.currentLevel.params.froggyThinkingTime);
    }
  }

  // jump if there is no vehicle on sight
  if (fstate.phase == InitJump) {
    if (!isAnyVehicleOnSight(fstate.nextLaneIndex, t.pos.x, t.pos.y, c.width * FROGGY_AI_RAY_FACTOR_TO_WIDTH_FOR_NO_JUMP)) {
      fstate.phase = DuringJump;
      fstate.phaseProgress = 0;
      fstate.jumpingFrom[X] = t.pos.x;
      fstate.jumpingFrom[Y] = t.pos.y;
      fstate.jumpingTo[X] = t.pos.x;
      fstate.jumpingTo[Y] = calcCenterYForLane(fstate.nextLaneIndex);
      fstate.nextLaneIndex -= fstate.yDirection;
    }
  }

  // it's currently on the fly
  if (fstate.phase == DuringJump) {
    fstate.phaseProgress += (deltaTime / FROGGY_JUMPING_TIME);

    vec3_lerp(t.pos.vec, fstate.jumpingFrom, fstate.jumpingTo, sin(PI / 2 * fstate.phaseProgress));
    t.pos.z = sin(PI * fstate.phaseProgress) * FROGGY_JUMP_AMPLITUDE;

    if (fstate.phaseProgress >= 1.0f) {
      fstate.phaseProgress = 0;
      fstate.phase = WaitForJump;
      t.pos.z = 0;

      if (fstate.nextLaneIndex == -2 || fstate.nextLaneIndex == state.currentLevel.params.laneCount + 1) {
        fstate.phase = Done;
      }
    }
  }

  // TODO record a state frame
END_ENTITY_SYSTEM

DEF_ENTITY_SYSTEM(CheckCollisions, A(Collider) | A(Transform))
  ref t1 = getCmpE(Transform);
  ref c1 = getCmpE(Collider);
  ref entity1Id = entity.id;
  float x1 = t1.pos.x - c1.width / 2;
  float y1 = t1.pos.y - c1.height / 2;

  FOR_EACH_ENTITY(world, A(Collider) | A(Transform))
    if (entity.id == entity1Id) {
      // don't check on yourself
      continue;
    }

    ref t2 = getCmpE(Transform);
    ref c2 = getCmpE(Collider);

    float x2 = t2.pos.x - c2.width / 2;
    float y2 = t2.pos.y - c2.height / 2;

    // we're doing 2d top-down collision check here, so ignoring 'z' coord
    if (x1 < x2 + c2.width &&
        x1 + c1.width > x2 &&
        y1 < y2 + c2.height &&
        y1 + c1.height > y2) {

      state.lastCollisionEntity1Id = entity1Id;
      state.lastCollisionEntity2Id = entity.id;
      goToPhase(ShowCollisionBeforeRewind);
    }
  END_FOR_EACH
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
               toRadian(froggy.state.yDirection == Up ? 0 : 180));

  if (froggy.state.phase == DuringJump) {
    mat4_rotateX(t.orientation, t.orientation, -cos(-PI / 2 + PI * (froggy.state.phaseProgress)) * PI_180 * 15.0f);
  }
END_ENTITY_SYSTEM

void goToPhase(Phase newPhase) {
  state.phase = newPhase;
  state.phaseTime = 0;

  if (newPhase == Simulate) {
    state.currentFrame = 0;
    state.recordedFrames.init(sizeof(RecordedFrame));
  }
  else if (newPhase == RewindAnimation) {
    state.rewindCurrentFrameDtLeft = 0;
    tweenFloat(state.tweens, REWIND_ANIM_SHADER_EFFECT_TURN_ON_DURATION, &state.camera.pos.z, 0.0f, 150.0f, Linear);
  }
  else if (newPhase == ShowCollisionBeforeRewind) {
  }
  else if (newPhase == Playing) {
    state.shaderRewind = 0;
    state.selectedVehicleEntityId = -1;

    if (state.ecsWorld.hasComponent<Vehicle>(state.lastCollisionEntity1Id))
      state.selectedVehicleEntityId = state.lastCollisionEntity1Id;
    else if (state.ecsWorld.hasComponent<Vehicle>(state.lastCollisionEntity2Id))
      state.selectedVehicleEntityId = state.lastCollisionEntity2Id;
  }
}

void initGame() {
  registerShaderUniform("rewind", SHADER_UNIFORM_1f, &state.shaderRewind);

  state.timeSinceLastRender = 0;
  memset(state.keyState, 0, sizeof(state.keyState) / 8);
  initTweens(state.tweens);
  state.levelGarbage.init(sizeof(void *));
  int sizes[] = COMPONENT_TYPE_SIZES;
  initEcsWorld(state.ecsWorld, sizes, COMPONENT_TYPE_COUNT);
  vec3_set(state.camera.pos.vec, 500, 0, 0);
  vec3_set(state.camera.dir.vec, 0, 0, -1);
  vec3_set(state.camera.rot.vec, 0, 0, toRadian(-90));

  initLevel(0);
  goToPhase(Simulate); // Intro;
  recordFrame(0);
}

bool onEvent(int eventType, int value) {
  if (eventType == EVENT_KEYDOWN || eventType == EVENT_KEYUP) {
    state.keyState[value] = eventType == EVENT_KEYDOWN;

    switch (value) {
    case KEY_RIGHT:
    case KEY_LEFT:
    case KEY_UP:
    case KEY_DOWN:
    case 'D':
    case 'A':
    case 'W':
    case 'S':
    case 107 /*KP_PLUS*/:
    case 187 /*plus*/:
    case 109 /*KP_MINUS*/:
    case 189 /*minus*/:
    case '-':
      return true;

    default:
      _l(value);
      return false;
    }
  }
  else {
    _l(value);
    return false;
  }

  return true;
}

void updateOnKeys() {
  ref s = state.keyState;
  if (s[KEY_RIGHT]) {
    if (state.phase == Playing) {
      loadFrame(state.currentFrame + 10);
    }
  }
  else if (s[KEY_LEFT]) {
    if (state.phase == Playing) {
      loadFrame(state.currentFrame - 10);
    }
  }
  if (s[KEY_UP]) {
  }
  else if (s[KEY_DOWN]) {
  }

  if (s[102] /*KP_RIGHT*/ || s['D']) {
    state.camera.pos.x += tw(0, 10);
  }
  else if (s[100] /*KP_LEFT*/ || s['A']) {
    state.camera.pos.x -= tw(0, 10);
  }
  if (s[104] /*KP_UP*/ || s['W']) {
    state.camera.pos.y -= tw(0, 10);
  }
  else if (s[98] /*KP_DOWN*/ || s['S']) {
    state.camera.pos.y += tw(0, 10);
  }
  if (s[107] /*KP_PLUS*/ || s[187] /*plus*/) {
    state.camera.pos.z -= tw(0, 10);
  }
  else if (s[109] /*KP_MINUS*/ || s[189] /*minus*/) {
    state.camera.pos.z += tw(0, 10);
  }
}

void renderGame(float deltaTime) {
  beginFrame();

  float canvasWidth = getCanvasWidth();
  float canvasHeight = getCanvasHeight();
  float h = canvasHeight - BOTTOM_UI_HEIGHT;
  float w = canvasWidth;
  float w2 = w / 2.0;
  float h2 = h / 2.0;

  const float aspectRatio = canvasWidth / canvasHeight;
  const float fieldOfView = 90.0 * PI_180;

  const float zNear = 0;

  updateOnKeys();

  // axis X goes right, axis Y goes down, axis Z goes towards viewer
  mat4_perspective(getProjectionMatrix(), fieldOfView, aspectRatio, 1, 2000.0);
  mat4_multiply(getProjectionMatrix(), getProjectionMatrix(), mat4_fromScaling(mat4Tmp, vec3_set(vec3Tmp, 1, -1, 1)));

  // top left point is 0,0; center is width/2,height/2
  mat4_translate(getViewMatrix(), mat4_identity(getViewMatrix()), vec3_set(vec3Tmp, -canvasWidth / 2, -canvasHeight / 2, -canvasHeight / 2.0));

  // TODO state.camera.dir
  float camZ = -state.camera.pos.z;
  auto matCam = getViewMatrix();
  mat4_rotateZ(matCam, matCam, state.camera.rot.z);
  mat4_translate(
      matCam,
      matCam,
      vec3_set(vec3Tmp, -state.camera.pos.x, -state.camera.pos.y, camZ));

  //TODO camera.dir

  // process logic with ECS World
  EcsWorld &world = state.ecsWorld;
  const float debugSpeedFactor = tw(2, 1.0f);
  world.deltaTime = deltaTime * debugSpeedFactor;

  Phase phase = state.phase;

  updateTweens(state.tweens, world.deltaTime);
  if (phase == Simulate) {
    SimulateVehicle(world);
    SimulateFroggy(world);
    CheckCollisions(world);
    if (phase == Simulate) {
      recordFrame(deltaTime);
    }
    _lstr("recorded frames", state.recordedFrames.size);
  }
  else if (phase == ShowCollisionBeforeRewind) {
    state.phaseTime += deltaTime * debugSpeedFactor;
    // TODO animate camera to show the collision! play some fail sound

    state.shaderRewind = CLAMP01(state.phaseTime / REWIND_ANIM_SHADER_EFFECT_TURN_ON_DURATION);

    if (state.phaseTime >= STALL_BEFORE_REWIND_DURATION) {
      goToPhase(RewindAnimation);
    }
  }
  else if (phase == RewindAnimation) {
    animateRewind(deltaTime, debugSpeedFactor);
  }
  else if (phase == Playing) {
    int goToFrame = (int)tw(1, 0);
    if (goToFrame >= 0) {
      loadFrame(goToFrame);
    }
    // TODO
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
    const float laneWidth = level.params.laneWidth;
    const float lanesGap = level.params.lanesGap;
    const float roadsideWidth = level.params.roadsideWidth;
    const float grassWidth = level.render.grassWidth;
    const float roadWidth = level.render.roadWidth;

    const float roadLength = w * 4;
    const float roadBegin = 0;
    const float roadEnd = roadLength;
    float roadX = 0;

    // grass - left
    texRect(TEXTURE_GRASS,
            roadBegin, roadX, z, roadLength, grassWidth,
            roadLength / grassWidth, 1.0f);

    roadX += grassWidth;

    // roadside left
    setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
    rect(roadBegin, roadX, z, roadLength, roadsideWidth);
    roadX += roadsideWidth;

    // black road
    setColorLeftToRight(1, 0.1, 0.1, 0.1, 0, 0, 0);
    rect(roadBegin, roadX, z, roadLength, roadWidth);

    // lane gaps - left to right
    float laneX = roadX + roadWidth;
    for (int i = 0; i < laneCount - 1; ++i) {
      laneX -= (laneWidth + lanesGap);

      setColor(1, 1, 1, 1);
      rect(roadBegin, laneX, z, roadLength, lanesGap);
    }
    roadX += roadWidth;

    // roadside right
    setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
    rect(roadBegin, roadX, z, roadLength, roadsideWidth);
    roadX += roadsideWidth;
    
    // grass - right
    texRect(TEXTURE_GRASS,
            roadBegin, roadX, z, roadLength, grassWidth,
            roadLength / grassWidth, 1.0f);
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

      if (state.phase == Playing && entity.id == state.selectedVehicleEntityId) {
        setColor(0.4, 1, 1, 1);
        rect(x, y, z, c.width, c.height);
      }

      // debug: front of car
      setColor(1, 1, 1, 0);
      rect(t.pos.x + lane.horzDir * c.width / 2, t.pos.y - c.height / 2, z, frontWidth, c.height);
    END_FOR_EACH
  }

  endFrame();
}

void render(float deltaTime) {
  state.timeSinceLastRender += deltaTime;
  while (state.timeSinceLastRender - TIME_PER_FRAME >= 0.0f) {
    state.timeSinceLastRender -= TIME_PER_FRAME;
    renderGame(TIME_PER_FRAME);
  }
}
