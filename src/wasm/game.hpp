#ifndef GAME__H
#define GAME__H

#define WASM_EXPORT __attribute__((used))

extern "C" {
  WASM_EXPORT int preinit();
  WASM_EXPORT void generateTextures();
  WASM_EXPORT int initGame();
  WASM_EXPORT void render(float deltaTime);

  extern float getCanvasWidth();
  extern float getCanvasHeight();
  extern float Math_tan(float a);
  extern void triggerDrawCall();
}


#endif