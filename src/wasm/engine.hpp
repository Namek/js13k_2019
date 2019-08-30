#ifndef ENGINE__H
#define ENGINE__H

#include "common.hpp"
#include "math/common.h"

extern "C" {
WASM_EXPORT int *preinit(void *memoryBase);
WASM_EXPORT void generateTextures();
WASM_EXPORT void initEngine();
WASM_EXPORT void render(float deltaTime);
WASM_EXPORT bool onEvent(int eventType, int value);
#ifndef PRODUCTION
WASM_EXPORT void setTweakValue(int index, float value);
#endif

extern float getCanvasWidth();
extern float getCanvasHeight();
extern float Math_tan(float a);
extern void clearFrame();
extern void triggerDrawCall();
extern void sendTexture(int ptr, int width, int height);
extern void registerShaderUniform(const char *name, uint type, void *ptr);
}

#define MAX_VIEW_MATRIX_COUNT 20
#define MAX_MODEL_MATRIX_COUNT 20

// clang-format off
struct EngineState {
  // shared memory
  int
   *funcReturn
  , funcReturn_size
  ,*renderIndexBuffer
  ;

  float
    *renderColorBuffer
  , *renderVertexBuffer
  , *renderTexCoordsBuffer
  , *renderNormalBuffer
  ,  projectionMatrix[MAT_SIZE_4]
  ,  viewMatrix[MAT_SIZE_4 * (MAX_VIEW_MATRIX_COUNT + 1)]
  ,  modelMatrix[MAT_SIZE_4 * (MAX_MODEL_MATRIX_COUNT + 1)]
  ,  normalMatrix[MAT_SIZE_4]

  // internal memory
  , *currentColor
  #ifndef PRODUCTION
  , tweakValues[10]
  #endif
  ;

  int
    vertexCount
  , indexCount
  , currentTextureId
  , currentViewMatrixIndex
  , currentModelMatrixIndex
  ;
};
extern EngineState engineState;
// clang-format on

void beginFrame();
void endFrame();
void flushBuffers();

const uint SHADER_UNIFORM_1i = 0;
const uint SHADER_UNIFORM_1f = 1;
const uint SHADER_UNIFORM_Matrix4fv = 2;
const uint SHADER_UNIFORM_OFFSET = 0;

const int EVENT_KEYDOWN = 1;
const int EVENT_KEYUP = 2;
const int KEY_LEFT = 37;
const int KEY_UP = 38;
const int KEY_RIGHT = 39;
const int KEY_DOWN = 40;
const int KEY_SPACE = 32;

#ifndef PRODUCTION
#define tw(index, defaultValue) engineState.tweakValues[index]
#else
#define tw(index, defaultValue) defaultValue
#endif

float *getProjectionMatrix();
float *getViewMatrix();
float *getModelMatrix();
void setProjectionMatrix(float *matrix);
void setViewMatrix(float *matrix);
void setModelMatrix(float *matrix);
float *pushViewMatrix();
void popViewMatrix();
float *pushModelMatrix();
void popModelMatrix();

void setColors3(float alpha, float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3);

void setColors4(
    float alpha,
    float r1, float g1, float b1,
    float r2, float g2, float b2,
    float r3, float g3, float b3,
    float r4, float g4, float b4);

void setColor(float alpha, float r, float g, float b);
void setColorLeftToRight(
    float alpha,
    float r1, float g1, float b1,
    float r2, float g2, float b2);

int vertex(float x, float y, float z);
void index(int i);
void calculateTriangleNormal(uint firstVertexIndex, uint faceSize);

void triangle(
    float v1x, float v1y, float v1z,
    float v2x, float v2y, float v2z,
    float v3x, float v3y, float v3z);

void texTriangle(
    int textureId,
    float v1x, float v1y, float v1z, float u1, float v1,
    float v2x, float v2y, float v2z, float u2, float v2,
    float v3x, float v3y, float v3z, float u3, float v3);

void quad(
    float v1x, float v1y, float v1z,
    float v2x, float v2y, float v2z,
    float v3x, float v3y, float v3z,
    float v4x, float v4y, float v4z);

void texQuad(
    int textureId,
    float v1x, float v1y, float v1z, float u1, float v1,
    float v2x, float v2y, float v2z, float u2, float v2,
    float v3x, float v3y, float v3z, float u3, float v3,
    float v4x, float v4y, float v4z, float u4, float v4);

int align(int x, int by);
int rgba(int r, int g, int b, int a);
int rgb(int r, int g, int b);

void rect(float x, float y, float z, float width, float height);
void texRect(int textureId, float x, float y, float z, float width, float height, float u1 = 1, float v1 = 1, float u0 = 0, float v0 = 0);

#endif