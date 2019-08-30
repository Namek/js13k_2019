#include "game.hpp"
#include "textures.hpp"
#include "models/models.hpp"

GameState state;
const float FROGGY_JUMPING_TIME = 0.4f;
const float FROGGY_JUMP_AMPLITUDE = 4.5f;
const float FROGGY_AI_RAY_FACTOR_TO_WIDTH_FOR_NO_JUMP = 2.5f;
const float STALL_BEFORE_REWIND_DURATION = 0.7f;
const float REWIND_TIME_FACTOR = 2.5f;
const float REWIND_ANIM_SHADER_EFFECT_TURN_ON_DURATION = 0.5f;
const float REWIND_ANIM_SHADER_EFFECT_TURN_OFF_DURATION = 0.75f;

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
      state.phase = ShowCollisionBeforeRewind;
      state.phaseTime = 0;
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

void recordFrame(float deltaTime);

void initGame() {
  registerShaderUniform("rewind", SHADER_UNIFORM_1f, &state.shaderRewind);

  state.levelGarbage.init(sizeof(void *));
  int sizes[] = COMPONENT_TYPE_SIZES;
  initEcsWorld(state.ecsWorld, sizes, COMPONENT_TYPE_COUNT);
  vec3_set(state.camera.pos.vec, 0, 0, 0);
  vec3_set(state.camera.dir.vec, 0, 0, -1);

  initLevel(0);
  state.phase = Simulate; // Intro;
  state.currentFrame = 0;
  state.recordedFrames.init(sizeof(RecordedFrame));
  recordFrame(0);
}

bool onEvent(int eventType, int value) {
  if (eventType != EVENT_KEYDOWN) {
    return false;
  }

  if (value == KEY_RIGHT) {
  }
  else if (value == KEY_LEFT) {
  }
  else if (value == KEY_UP) {
  }
  else if (value == KEY_RIGHT) {
  }
  else if (value == 102 /*KP_RIGHT*/) {
    state.camera.pos.x += tw(0, 10);
  }
  else if (value == 100 /*KP_LEFT*/) {
    state.camera.pos.x -= tw(0, 10);
  }
  else if (value == 104 /*KP_UP*/) {
    state.camera.pos.y -= tw(0, 10);
  }
  else if (value == 98 /*KP_DOWN*/) {
    state.camera.pos.y += tw(0, 10);
  }
  else if (value == 107 /*KP_PLUS*/) {
    state.camera.pos.z -= tw(0, 10);
  }
  else if (value == 109 /*KP_MINUS*/) {
    state.camera.pos.z += tw(0, 10);
  }
  else {
    _l(value);
    return false;
  }

  return true;
}

void recordFrame(float deltaTime) {
  ref world = state.ecsWorld;

  FOR_EACH_ENTITY(world, A(Vehicle) | A(Transform))
    ref v = getCmpE(Vehicle);
    ref vt = getCmpE(Transform);

    auto newFrame = (VehicleTimeFrame *)v.paramsDynamicSimulatedFrames.createPtrAt(state.currentFrame);
    newFrame->transform = vt;
    newFrame->params = v.paramsDynamicCurrent;
  END_FOR_EACH

  FOR_EACH_ENTITY(world, A(Froggy) | A(Transform))
    ref frog = getCmpE(Froggy);
    ref t = getCmpE(Transform);
    ref fstate = frog.state;

    auto newFrame = (FroggyTimeFrame *)frog.simulatedFrames.createPtrAt(state.currentFrame);
    newFrame->transform = t;
    newFrame->state = fstate;
  END_FOR_EACH

  auto frame = (RecordedFrame *)state.recordedFrames.createPtr();
  frame->deltaTime = deltaTime;

  float totalTime = 0;
  if (state.currentFrame > 0) {
    auto prevFrame = (RecordedFrame *)state.recordedFrames.getPointer(state.currentFrame - 1);
    totalTime = prevFrame->totalTime;
  }
  frame->totalTime = totalTime + deltaTime;

  state.currentFrame += 1;
}

RecordedFrame *loadFrame(int frameIndex) {
  if (frameIndex >= state.recordedFrames.size)
    return NULL;

  state.currentFrame = frameIndex;
  _lstr("load/frame", state.currentFrame);

  ref world = state.ecsWorld;

  FOR_EACH_ENTITY(world, A(Vehicle) | A(Transform))
    ref v = getCmpE(Vehicle);
    ref vt = getCmpE(Transform);

    auto frame = (VehicleTimeFrame *)v.paramsDynamicSimulatedFrames.getPointer(state.currentFrame);
    vt.pos = frame->transform.pos;
    memcpy(frame->transform.orientation, vt.orientation, MAT_SIZE_4 * sizeof(float));
    v.paramsDynamicCurrent = frame->params;
  END_FOR_EACH

  FOR_EACH_ENTITY(world, A(Froggy) | A(Transform))
    ref frog = getCmpE(Froggy);
    ref ft = getCmpE(Transform);
    ref fstate = frog.state;

    auto frame = (FroggyTimeFrame *)frog.simulatedFrames.getPointer(state.currentFrame);
    ft.pos = frame->transform.pos;
    memcpy(frame->transform.orientation, ft.orientation, MAT_SIZE_4 * sizeof(float));
    frog.state = frame->state;
    _lfstr("load/frog.pos.y", frame->transform.pos.y);
  END_FOR_EACH

  return (RecordedFrame *)state.recordedFrames.getPointer(frameIndex);
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
  mat4_perspective(getProjectionMatrix(), fieldOfView, aspectRatio, 1, 2000.0);
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
  const float debugSpeedFactor = tw(2, 1.0f);
  world.deltaTime = deltaTime * debugSpeedFactor;

  Phase phase = state.phase;

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

    if (state.phaseTime >= STALL_BEFORE_REWIND_DURATION) {
      state.rewindCurrentFrameDtLeft = 0;
      state.phase = RewindAnimation;
      state.phaseTime = 0;
    }
  }
  else if (phase == RewindAnimation) {
    float dtLeft = (state.rewindCurrentFrameDtLeft + deltaTime) * REWIND_TIME_FACTOR * debugSpeedFactor;

    uint frameIndex = state.currentFrame;
    while (dtLeft >= 0 && frameIndex > 0) {
      auto frame = (RecordedFrame *)state.recordedFrames.getPtr(frameIndex--);
      dtLeft -= frame->deltaTime;
    }
    state.rewindCurrentFrameDtLeft = -dtLeft;
    auto frame = loadFrame(frameIndex);

    if (frameIndex == 0) {
      state.phase = Playing;
      state.shaderRewind = 0;
      state.phaseTime = 0;
    }
    else {
      float leftTime = frame->totalTime / (REWIND_TIME_FACTOR * debugSpeedFactor);

      if (leftTime > REWIND_ANIM_SHADER_EFFECT_TURN_OFF_DURATION) {
        state.phaseTime += deltaTime;
        state.shaderRewind = CLAMP01(state.phaseTime / REWIND_ANIM_SHADER_EFFECT_TURN_ON_DURATION);
      }
      else {
        // turn the shader down
        state.shaderRewind = CLAMP01(leftTime / REWIND_ANIM_SHADER_EFFECT_TURN_OFF_DURATION);
      }
    }
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
    const float laneHeight = level.params.laneHeight;
    const float lanesGap = level.params.lanesGap;
    const float roadsideHeight = level.params.roadsideHeight;
    const float grassHeight = level.render.grassHeight;
    const float roadHeight = level.render.roadHeight;

    const float roadWidth = w * 4;
    const float roadLeft = -roadWidth / 2 + w2;
    const float roadRight = roadWidth / 2 + w2;

    // grass - top
    texQuad(TEXTURE_GRASS,
            roadLeft, 0, z, 0, 0,
            roadRight, 0, z, roadWidth / grassHeight, 0,
            roadRight, grassHeight, z, roadWidth / grassHeight, 1,
            roadLeft, grassHeight, z, 0, 1);

    // grass - bottom
    texRect(TEXTURE_GRASS,
            roadLeft, h - grassHeight, z, roadWidth, grassHeight,
            roadWidth / grassHeight, 1.0f);

    float roadY = grassHeight + roadsideHeight;

    // roadsides
    setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
    rect(roadLeft, roadY - roadsideHeight, z, roadWidth, roadsideHeight);
    rect(roadLeft, roadY + roadHeight, z, roadWidth, roadsideHeight);

    // black road
    setColorLeftToRight(1, 0.1, 0.1, 0.1, 0, 0, 0);
    rect(roadLeft, roadY, z, roadWidth, roadHeight);

    // lane gaps - top to bottom
    float laneY = roadY + roadHeight;
    for (int i = 0; i < laneCount - 1; ++i) {
      laneY -= (laneHeight + lanesGap);

      setColor(1, 1, 1, 1);
      rect(roadLeft, laneY, z, roadWidth, lanesGap);
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

      if (state.phase == Playing) {
        // TODO special effects of rewinding
      }
    END_FOR_EACH
  }

  endFrame();
}