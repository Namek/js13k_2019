#include "game.hpp"

const int MAX_TRIANGLES = 1024;

// shared memory
const int SIZE_FUNC_RETURN = 100;

const int INT_SIZE = 4, FLOAT_SIZE = 4;
const int VALUES_PER_VERTEX = 3;
const int VALUES_PER_COLOR = 4;
const int VALUES_PER_TEXCOORD = 2;
const int COLOR_BYTES_PER_TRIANGLE = VALUES_PER_COLOR * VALUES_PER_VERTEX * 4;
const int BYTES_PER_VERTEX = VALUES_PER_VERTEX * 4;

const int SIZE_RENDER_COLOR_BUFFER = MAX_TRIANGLES * COLOR_BYTES_PER_TRIANGLE;
const int SIZE_RENDER_VERTEX_BUFFER = MAX_TRIANGLES * BYTES_PER_VERTEX;
const int SIZE_RENDER_INDEX_BUFFER = MAX_TRIANGLES * BYTES_PER_VERTEX; //usually, it's less
const int SIZE_RENDER_TEXCOORDS_BUFFER = MAX_TRIANGLES * BYTES_PER_VERTEX / 2;

const int SIZE_PROJECTION_MATRIX = 16 * FLOAT_SIZE;
const int SIZE_VIEW_MATRIX = 16 * FLOAT_SIZE;

int OFFSET_FUNC_RETURN = 0;
int OFFSET_RENDER_COLOR_BUFFER = 0;
int OFFSET_RENDER_VERTEX_BUFFER = 0;
int OFFSET_RENDER_INDEX_BUFFER = 0;
int OFFSET_RENDER_TEXCOORDS_BUFFER = 0;
int OFFSET_PROJECTION_MATRIX = 0;
int OFFSET_VIEW_MATRIX = 0;

// internal memory
int OFFSET_CURRENT_COLOR = 0;

const float I = 1.0;
const float O = 0.0;
const float PI = 3.141592653589793;
const float PI_180 = 0.017453292519943295; // PI/180.0

const int ALL_TEXTURES = 1;
const int TEXTURE_CHECKERBOARD = 0;

int preinit() {
  return SIZE_FUNC_RETURN;
}

// returns memory layout
void initGame() {
  int *ret = (int *)OFFSET_FUNC_RETURN;

  OFFSET_RENDER_COLOR_BUFFER = OFFSET_FUNC_RETURN + SIZE_FUNC_RETURN;
  OFFSET_RENDER_VERTEX_BUFFER = OFFSET_RENDER_COLOR_BUFFER + SIZE_RENDER_COLOR_BUFFER;
  OFFSET_RENDER_INDEX_BUFFER = OFFSET_RENDER_VERTEX_BUFFER + SIZE_RENDER_VERTEX_BUFFER;
  OFFSET_RENDER_TEXCOORDS_BUFFER = OFFSET_RENDER_INDEX_BUFFER + SIZE_RENDER_INDEX_BUFFER;
  OFFSET_PROJECTION_MATRIX = OFFSET_RENDER_TEXCOORDS_BUFFER + SIZE_RENDER_TEXCOORDS_BUFFER;
  OFFSET_VIEW_MATRIX = OFFSET_PROJECTION_MATRIX + SIZE_PROJECTION_MATRIX;

  const int OFFSET_SHARED_MEMORY_END = OFFSET_VIEW_MATRIX + SIZE_VIEW_MATRIX;

  // internal memory
  OFFSET_CURRENT_COLOR = OFFSET_SHARED_MEMORY_END;

  ret[0] = VALUES_PER_COLOR;
  ret[1] = VALUES_PER_VERTEX;
  ret[2] = SIZE_RENDER_COLOR_BUFFER;
  ret[3] = SIZE_RENDER_VERTEX_BUFFER;
  ret[4] = SIZE_RENDER_INDEX_BUFFER;
  ret[5] = SIZE_RENDER_TEXCOORDS_BUFFER;
  ret[6] = OFFSET_RENDER_COLOR_BUFFER;
  ret[7] = OFFSET_RENDER_VERTEX_BUFFER;
  ret[8] = OFFSET_RENDER_INDEX_BUFFER;
  ret[9] = OFFSET_RENDER_TEXCOORDS_BUFFER;
  ret[10] = OFFSET_PROJECTION_MATRIX;
  ret[11] = OFFSET_VIEW_MATRIX;
}

int rgba(int r, int g, int b, int a) {
  return (a << 24) | (b << 16) | (g << 8) | r;
}

inline int rgb(int r, int g, int b) {
  return rgba(r, g, b, 255);
}

// call it before init()
void generateTextures() {
  int *ret = (int *)OFFSET_FUNC_RETURN;
  int x = 0;
  int y = 0;
  int i = 0;
  int j = 0;
  int p = 0;

  int textureStart = OFFSET_FUNC_RETURN + SIZE_FUNC_RETURN + 1 + 3 * ALL_TEXTURES; //make some space for info about textures
  int *tex = (int *)textureStart;
  int textureCount = 0;

  const int w = 500;
  const int h = 500;
  const int gap = 50;
  int color = 0;

  // checkerboard: TEXTURE_CHECKERBOARD
  p = 0;
  for (y = 0; y < h; y += gap) {
    for (x = 0; x < w; x += gap) {
      // fill rectangle
      for (i = 0; i < gap; i += 1) {
        for (j = 0; j < gap; j += 1) {
          p = (y + i) * w + x + j;
          if (color == 0) {
            if (y < h / 2)
              tex[p] = rgb(255, 0, 0);
            else
              tex[p] = rgb(0, 255, 0);
          }
          else {
            tex[p] = rgb(255, 255, 255);
          }
        }
      }

      color = (color + 1) % 2;
    }

    // start row with different color than row above
    color = (color + 1) % 2;
  }

  textureCount += 1;

  // return info about all textures
  ret[0] = textureCount;
  ret[1] = w;
  ret[2] = h;
  ret[3] = textureStart;
}

void simulate() {
}

int vertexCount = 0;
int indexCount = 0;
int currentTextureId = 0;

void doDrawCall() {
  int *ret = (int *)OFFSET_FUNC_RETURN;

  ret[0] = vertexCount;
  ret[1] = indexCount;
  ret[2] = currentTextureId;

  triggerDrawCall();

  currentTextureId = -1;
  vertexCount = 0;
  indexCount = 0;
}

void setTexture(int newTextureId) {
  if (currentTextureId != newTextureId) {
    if (vertexCount > 0)
      doDrawCall();

    currentTextureId = newTextureId;
  }
}

void triangle(
    float v1x, float v1y, float v1z,
    float v2x, float v2y, float v2z,
    float v3x, float v3y, float v3z) {
  setTexture(-1);

  float *vertices = (float *)OFFSET_RENDER_VERTEX_BUFFER;
  float *colors = (float *)OFFSET_RENDER_COLOR_BUFFER;
  int *indices = (int *)OFFSET_RENDER_INDEX_BUFFER;
  float *currentColor = (float *)OFFSET_CURRENT_COLOR;

  int i = vertexCount * VALUES_PER_VERTEX;
  vertices[i] = v1x;
  vertices[i + 1] = v1y;
  vertices[i + 2] = v1z;
  vertices[i + 3] = v2x;
  vertices[i + 4] = v2y;
  vertices[i + 5] = v2z;
  vertices[i + 6] = v3x;
  vertices[i + 7] = v3y;
  vertices[i + 8] = v3z;

  i = vertexCount * VALUES_PER_COLOR;
  int j = 0;
  for (j = 0; j < 12; j += 1) {
    colors[i + j] = currentColor[j];
  }

  indices[indexCount] = vertexCount;
  indices[indexCount + 1] = vertexCount + 1;
  indices[indexCount + 2] = vertexCount + 2;

  vertexCount += 3;
  indexCount += 3;
}

void texTriangle(
    int textureId,
    float v1x, float v1y, float v1z, float u1, float v1,
    float v2x, float v2y, float v2z, float u2, float v2,
    float v3x, float v3y, float v3z, float u3, float v3) {
  setTexture(textureId);

  float *vertices = (float *)OFFSET_RENDER_VERTEX_BUFFER;
  float *texCoords = (float *)OFFSET_RENDER_TEXCOORDS_BUFFER;
  int *indices = (int *)OFFSET_RENDER_INDEX_BUFFER;

  int i = vertexCount * VALUES_PER_VERTEX;
  vertices[i] = v1x;
  vertices[i + 1] = v1y;
  vertices[i + 2] = v1z;
  vertices[i + 3] = v2x;
  vertices[i + 4] = v2y;
  vertices[i + 5] = v2z;
  vertices[i + 6] = v3x;
  vertices[i + 7] = v3y;
  vertices[i + 8] = v3z;

  i = vertexCount * VALUES_PER_TEXCOORD;
  texCoords[i] = u1;
  texCoords[i + 1] = v1;
  texCoords[i + 2] = u2;
  texCoords[i + 3] = v2;
  texCoords[i + 4] = u3;
  texCoords[i + 5] = v3;

  indices[indexCount] = vertexCount;
  indices[indexCount + 1] = vertexCount + 1;
  indices[indexCount + 2] = vertexCount + 2;

  vertexCount += 3;
  indexCount += 3;
}

void quad(
    float v1x, float v1y, float v1z,
    float v2x, float v2y, float v2z,
    float v3x, float v3y, float v3z,
    float v4x, float v4y, float v4z) {
  //1,2,3
  triangle(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z);

  //3,4,1
  float *vertices = (float *)OFFSET_RENDER_VERTEX_BUFFER;
  float *colors = (float *)OFFSET_RENDER_COLOR_BUFFER;
  int *indices = (int *)OFFSET_RENDER_INDEX_BUFFER;
  float *currentColor = (float *)OFFSET_CURRENT_COLOR;

  int i = vertexCount * VALUES_PER_VERTEX;
  vertices[i] = v4x;
  vertices[i + 1] = v4y;
  vertices[i + 2] = v4z;

  i = vertexCount * VALUES_PER_COLOR;
  colors[i] = currentColor[12];
  colors[i + 1] = currentColor[13];
  colors[i + 2] = currentColor[14];
  colors[i + 3] = currentColor[15];

  indices[indexCount] = vertexCount - 1;
  indices[indexCount + 1] = vertexCount;
  indices[indexCount + 2] = vertexCount - 3;

  vertexCount += 1;
  indexCount += 3;
}

void texQuad(
    int textureId,
    float v1x, float v1y, float v1z, float u1, float v1,
    float v2x, float v2y, float v2z, float u2, float v2,
    float v3x, float v3y, float v3z, float u3, float v3,
    float v4x, float v4y, float v4z, float u4, float v4) {
  //1,2,3
  texTriangle(textureId,
              v1x, v1y, v1z, u1, v1,
              v2x, v2y, v2z, u2, v2,
              v3x, v3y, v3z, u3, v3);

  //3,4,1
  float *vertices = (float *)OFFSET_RENDER_VERTEX_BUFFER;
  float *texCoords = (float *)OFFSET_RENDER_TEXCOORDS_BUFFER;
  int *indices = (int *)OFFSET_RENDER_INDEX_BUFFER;

  int i = vertexCount * VALUES_PER_VERTEX;
  vertices[i] = v4x;
  vertices[i + 1] = v4y;
  vertices[i + 2] = v4z;

  i = vertexCount * VALUES_PER_TEXCOORD;
  texCoords[i] = u4;
  texCoords[i + 1] = v4;

  indices[indexCount] = vertexCount - 1;
  indices[indexCount + 1] = vertexCount;
  indices[indexCount + 2] = vertexCount - 3;

  vertexCount += 1;
  indexCount += 3;
}

// set 3 colors for 3 triangle vertices
void setColors3(float alpha, float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3) {
  float *color = (float *)OFFSET_CURRENT_COLOR;

  color[0] = r1;
  color[1] = g1;
  color[2] = b1;
  color[3] = alpha;
  color[4] = r2;
  color[5] = g2;
  color[6] = b2;
  color[7] = alpha;
  color[8] = r3;
  color[9] = g3;
  color[10] = b3;
  color[11] = alpha;
}

void setColors4(
    float alpha,
    float r1, float g1, float b1,
    float r2, float g2, float b2,
    float r3, float g3, float b3,
    float r4, float g4, float b4) {
  float *color = (float *)OFFSET_CURRENT_COLOR;
  setColors3(alpha, r1, g1, b1, r2, g2, b2, r3, g3, b3);

  color[12] = r4;
  color[13] = g4;
  color[14] = b4;
  color[15] = alpha;
}

// set single color for 3 triangle vertices
void setColor(float alpha, float r, float g, float b) {
  float *color = (float *)OFFSET_CURRENT_COLOR;

  int i = 0;
  for (i = 0; i < 16; i += 4) {
    color[i] = r;
    color[i + 1] = g;
    color[i + 2] = b;
    color[i + 3] = alpha;
  }
}

void set16f(
    float *a,
    float f0, float f1, float f2, float f3,
    float f4, float f5, float f6, float f7,
    float f8, float f9, float f10, float f11,
    float f12, float f13, float f14, float f15) {
  a[0] = f0;
  a[1] = f1;
  a[2] = f2;
  a[3] = f3;
  a[4] = f4;
  a[5] = f5;
  a[6] = f6;
  a[7] = f7;
  a[8] = f8;
  a[9] = f9;
  a[10] = f10;
  a[11] = f11;
  a[12] = f12;
  a[13] = f13;
  a[14] = f14;
  a[15] = f15;
}

void setProjectionMatrix(
    float *addr,
    float fieldOfViewInRadians, float aspect, float near, float far) {
  const float f = 1; //Math_tan(PI * 0.5f - 0.5f * fieldOfViewInRadians);
  const float rangeInv = 1.0 / (near - far);

  set16f(addr,
         f / aspect, O, O, O,
         O, f, O, O,
         O, O, (near + far) * rangeInv, -1.0,
         O, O, near * far * rangeInv * 2.0, O);
}

// returns numbers:
//  - number of vertices (same as number of colors)
//  - number of indices
void render(float deltaTime) {
  float width = getCanvasWidth();
  float height = getCanvasHeight();
  float h = height;
  float w = width;
  float w2 = w / 2.0;
  float h2 = h / 2.0;

  vertexCount = 0;
  indexCount = 0;
  currentTextureId = -1;

  const float aspectRatio = width / height;
  const float fieldOfViewInDegrees = 90.0 * PI_180;
  setProjectionMatrix((float *)OFFSET_PROJECTION_MATRIX, fieldOfViewInDegrees, aspectRatio, 1.0, 2000.0);

  // bottom left point is 0,0; center is width/2,height/2
  const float cameraZ = height / 2.0;
  set16f((float *)OFFSET_VIEW_MATRIX,
         1.0, 0.0, 0.0, 0.0,
         0.0, 1.0, 0.0, 0.0,
         0.0, 0.0, 1.0, 0.0,
         -w2, -h2, -cameraZ, 1.0);

  // background
  const float z = 0;
  setColor(I, O, O, O);

  setColors4(
      1.0,
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0,
      I, 0.5, 0.0);

  quad(
      O, O, z,
      O, h2, z,
      w2 / 2.0, h2, z,
      width, O, z);
  quad(
      O, O, z,
      O, h2, z,
      w2 / 2.0, h2, z,
      width, O, z);

  texTriangle(TEXTURE_CHECKERBOARD,
              w2, h, O, O, O,
              w2, h / 3, O, O, 1.5,
              w, h / 3, O, w / h, 1.5);

  setColors3(1.0,
             1.0, 0.0, 0.0,
             0.0, 1.0, 0.0,
             0.0, 0.0, 1.0);

  int i = 0;
  for (i = 0; i < 8; i += 1) {
    const float m = i * 90;
    triangle(
        w2 - m, h2, 0.0,
        w2 - m, O, 0.0,
        w - m, O, 0.0);
  }

  doDrawCall();
}
