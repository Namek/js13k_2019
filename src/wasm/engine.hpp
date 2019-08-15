#ifndef ENGINE__H
#define ENGINE__H

#define WASM_EXPORT __attribute__((used))

extern "C" {
WASM_EXPORT int preinit();
WASM_EXPORT void generateTextures();
WASM_EXPORT void initEngine();
WASM_EXPORT void render(float deltaTime);

extern float getCanvasWidth();
extern float getCanvasHeight();
extern float Math_tan(float a);
extern float random();
extern void triggerDrawCall();
extern void l(int); //logging
}

void beginFrame();
void endFrame();
void doDrawCall();

const float I = 1.0;
const float O = 0.0;
const float PI = 3.141592653589793;
const float PI_180 = 0.017453292519943295; // PI/180.0

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(a, min, max) MIN(max, MAX(min, a))
#define CLAMP255(a) (int)(CLAMP(a, 0, 255))
#define CLAMP01(a) (CLAMP(a, 0f, 1f))

void set16f(
    float *a,
    float f0, float f1, float f2, float f3,
    float f4, float f5, float f6, float f7,
    float f8, float f9, float f10, float f11,
    float f12, float f13, float f14, float f15);

void setProjectionMatrix(
    float fieldOfViewInRadians, float aspect, float near, float far);

void setViewMatrix(
    float f0, float f1, float f2, float f3,
    float f4, float f5, float f6, float f7,
    float f8, float f9, float f10, float f11,
    float f12, float f13, float f14, float f15);

void setColors3(float alpha, float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3);

void setColors4(
    float alpha,
    float r1, float g1, float b1,
    float r2, float g2, float b2,
    float r3, float g3, float b3,
    float r4, float g4, float b4);

void setColor(float alpha, float r, float g, float b);

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


#endif