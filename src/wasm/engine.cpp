#include "engine.hpp"
#include "utils/memory.hpp"

EngineState engineState;
#define e (engineState)

const int MAX_TRIANGLES = 1024;

// shared memory
const int SIZE_FUNC_RETURN = 100;

const int INT_SIZE = 4, FLOAT_SIZE = 4, MATRIX_SIZE = 16 * FLOAT_SIZE;
const int VALUES_PER_VERTEX = 3;
const int VALUES_PER_COLOR = 4;
const int VALUES_PER_TEXCOORD = 2;
const int COLOR_BYTES_PER_TRIANGLE = VALUES_PER_COLOR * VALUES_PER_VERTEX * 4;
const int BYTES_PER_VERTEX = VALUES_PER_VERTEX * 4;

const int SIZE_RENDER_COLOR_BUFFER = MAX_TRIANGLES * COLOR_BYTES_PER_TRIANGLE;
const int SIZE_RENDER_VERTEX_BUFFER = MAX_TRIANGLES * BYTES_PER_VERTEX;
const int SIZE_RENDER_NORMAL_BUFFER = SIZE_RENDER_VERTEX_BUFFER;
const int SIZE_RENDER_INDEX_BUFFER = SIZE_RENDER_VERTEX_BUFFER; //usually it's less
const int SIZE_RENDER_TEXCOORDS_BUFFER = SIZE_RENDER_VERTEX_BUFFER / 2;


// internal memory
const int SIZE_CURRENT_COLOR = 16 * sizeof(float);

int *preinit(void *heapStart) {
  e.funcReturn = (int *)heapStart;
  e.funcReturn_size = SIZE_FUNC_RETURN;
  return e.funcReturn;
}

#define FLOAT_PTR(from, size) (float *)(((char *)from) + size)
#define INT_PTR(from, size) (int *)(((char *)from) + size)

// set ups memory layout
void initEngine() {
  int *ret = e.funcReturn;

  e.renderColorBuffer = FLOAT_PTR(e.funcReturn, e.funcReturn_size);
  e.renderVertexBuffer = FLOAT_PTR(e.renderColorBuffer, SIZE_RENDER_COLOR_BUFFER);
  e.renderIndexBuffer = INT_PTR(e.renderVertexBuffer, SIZE_RENDER_VERTEX_BUFFER);
  e.renderTexCoordsBuffer = FLOAT_PTR(e.renderIndexBuffer, SIZE_RENDER_INDEX_BUFFER);
  e.renderNormalBuffer = FLOAT_PTR(e.renderTexCoordsBuffer, SIZE_RENDER_TEXCOORDS_BUFFER);

  // internal memory
  e.currentColor = FLOAT_PTR(e.renderNormalBuffer, SIZE_RENDER_NORMAL_BUFFER);
  const int OFFSET_DYNAMIC_MEMORY = align((int)e.currentColor + SIZE_CURRENT_COLOR, 16);

  ret[0] = VALUES_PER_COLOR;
  ret[1] = VALUES_PER_VERTEX;
  ret[2] = SIZE_RENDER_COLOR_BUFFER;
  ret[3] = SIZE_RENDER_VERTEX_BUFFER;
  ret[4] = SIZE_RENDER_NORMAL_BUFFER;
  ret[5] = SIZE_RENDER_INDEX_BUFFER;
  ret[6] = SIZE_RENDER_TEXCOORDS_BUFFER;
  ret[7] = (int)e.renderColorBuffer;
  ret[8] = (int)e.renderVertexBuffer;
  ret[9] = (int)e.renderIndexBuffer;
  ret[10] = (int)e.renderTexCoordsBuffer;
  ret[11] = (int)e.renderNormalBuffer;
  ret[12] = OFFSET_DYNAMIC_MEMORY;

  registerShaderUniform("projMat", SHADER_UNIFORM_Matrix4fv, e.projectionMatrix);
  registerShaderUniform("viewMat", SHADER_UNIFORM_Matrix4fv, e.viewMatrix);
  registerShaderUniform("mdlMat", SHADER_UNIFORM_Matrix4fv, e.modelMatrix);
  registerShaderUniform("norMat", SHADER_UNIFORM_Matrix4fv, e.normalMatrix);
}

void beginFrame() {
  e.vertexCount = 0;
  e.indexCount = 0;
  e.currentTextureId = -1;
  e.currentViewMatrixIndex = MAX_VIEW_MATRIX_COUNT;
  e.currentModelMatrixIndex = MAX_MODEL_MATRIX_COUNT;
  mat4_identity(getViewMatrix());
  mat4_identity(getModelMatrix());

  clearFrame();
}

void endFrame() {
  if (e.indexCount > 0)
    flushBuffers();
}

void flushBuffers() {
  if (e.indexCount == 0)
    return;

  int *ret = e.funcReturn;

  ret[0] = e.vertexCount;
  ret[1] = e.indexCount;
  ret[2] = e.currentTextureId;

  memcpy(
      e.viewMatrix,
      FLOAT_PTR(e.viewMatrix, e.currentViewMatrixIndex * MATRIX_SIZE),
      MATRIX_SIZE);

  memcpy(
      e.modelMatrix,
      FLOAT_PTR(e.modelMatrix, e.currentModelMatrixIndex * MATRIX_SIZE),
      MATRIX_SIZE);

  mat4_invert(e.normalMatrix, mat4_multiply(mat4Tmp, e.modelMatrix, e.viewMatrix));
  mat4_transpose(e.normalMatrix, e.normalMatrix);

  triggerDrawCall();

  e.currentTextureId = -1;
  e.vertexCount = 0;
  e.indexCount = 0;
}

void setTexture(int newTextureId) {
  if (e.currentTextureId != newTextureId) {
    flushBuffers();
    e.currentTextureId = newTextureId;
  }
}

// set 3 colors for 3 triangle vertices
void setColors3(float alpha, float r1, float g1, float b1, float r2, float g2, float b2, float r3, float g3, float b3) {
  float *color = e.currentColor;

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
  float *color = (float *)e.currentColor;
  setColors3(alpha, r1, g1, b1, r2, g2, b2, r3, g3, b3);

  color[12] = r4;
  color[13] = g4;
  color[14] = b4;
  color[15] = alpha;
}

// set single color for 3 triangle vertices
void setColor(float alpha, float r, float g, float b) {
  float *color = e.currentColor;

  int i = 0;
  for (i = 0; i < 16; i += 4) {
    color[i] = r;
    color[i + 1] = g;
    color[i + 2] = b;
    color[i + 3] = alpha;
  }
}

// for rect(), usually
void setColorLeftToRight(
    float alpha,
    float r1, float g1, float b1,
    float r2, float g2, float b2) {
  setColors4(
      alpha,
      r1, g1, b1,
      r2, g2, b2,
      r2, g2, b2,
      r1, g1, b1);
}

float *getProjectionMatrix() {
  return e.projectionMatrix;
}

float *getViewMatrix() {
  return FLOAT_PTR(e.viewMatrix, e.currentViewMatrixIndex * MATRIX_SIZE);
}

float *getModelMatrix() {
  return FLOAT_PTR(e.modelMatrix, e.currentModelMatrixIndex * MATRIX_SIZE);
}

void setProjectionMatrix(float *matrix) {
  flushBuffers();

  float *dst = getProjectionMatrix();
  memcpy(dst, matrix, MATRIX_SIZE);
}

void setViewMatrix(float *matrix) {
  flushBuffers();

  float *dst = getViewMatrix();
  memcpy(dst, matrix, MATRIX_SIZE);
}

void setModelMatrix(float *matrix) {
  flushBuffers();

  float *dst = getModelMatrix();
  memcpy(dst, matrix, MATRIX_SIZE);
}

float *pushViewMatrix() {
  float *src = getViewMatrix();
  e.currentViewMatrixIndex -= 1;
  float *dst = getViewMatrix();
  memcpy(dst, src, MATRIX_SIZE);
  return dst;
}

void popViewMatrix() {
  flushBuffers();
  e.currentViewMatrixIndex += 1;
}

float *pushModelMatrix() {
  flushBuffers();
  float *src = getModelMatrix();
  e.currentModelMatrixIndex -= 1;
  float *dst = getModelMatrix();
  memcpy(dst, src, MATRIX_SIZE);
  return dst;
}

void popModelMatrix() {
  flushBuffers();
  e.currentModelMatrixIndex += 1;
}

// does not reset the texture
int vertex(float x, float y, float z) {
  int vertexCount = e.vertexCount;

  int i = vertexCount * VALUES_PER_VERTEX;
  e.renderVertexBuffer[i] = x;
  e.renderVertexBuffer[i + 1] = y;
  e.renderVertexBuffer[i + 2] = z;

  i = vertexCount * VALUES_PER_COLOR;
  int j = 0;
  for (j = 0; j < 4; j += 1) {
    e.renderColorBuffer[i + j] = e.currentColor[j];
  }

  e.vertexCount += 1;
  return vertexCount;
}

void index(int i) {
  e.renderIndexBuffer[e.indexCount] = i;
  e.indexCount += 1;
}

// vertices should
void calculateTriangleNormal(uint firstVertexIndex, uint faceSize) {
  static Vec3 u;
  static Vec3 v;
  uint i = firstVertexIndex * VALUES_PER_VERTEX;
  float *faceVertices = e.renderVertexBuffer + i;
  vec3_set(u.vec, faceVertices[3] - faceVertices[0], faceVertices[4] - faceVertices[1], faceVertices[5] - faceVertices[2]); //p2 - p1
  vec3_set(v.vec, faceVertices[6] - faceVertices[0], faceVertices[7] - faceVertices[1], faceVertices[8] - faceVertices[2]); //p3 - p1
  vec3_cross(u.vec, u.vec, v.vec);
  float len = sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
  vec3_scale(u.vec, u.vec, 1.0f / len);

  // _lfstr("nor.x", u.x);
  // _lfstr("nor.y", u.y);
  // _lfstr("nor.z", u.z);

  for (int j = 0; j < faceSize; ++j) {
    for (int k = 0; k < 3; ++k) {
      e.renderNormalBuffer[i + j * 3 + k] = u.vec[k];
    }
  }
}

void triangle(
    float v1x, float v1y, float v1z,
    float v2x, float v2y, float v2z,
    float v3x, float v3y, float v3z) {
  setTexture(-1);

  int vertexCount = e.vertexCount;

  int i = vertexCount * VALUES_PER_VERTEX;
  e.renderVertexBuffer[i] = v1x;
  e.renderVertexBuffer[i + 1] = v1y;
  e.renderVertexBuffer[i + 2] = v1z;
  e.renderVertexBuffer[i + 3] = v2x;
  e.renderVertexBuffer[i + 4] = v2y;
  e.renderVertexBuffer[i + 5] = v2z;
  e.renderVertexBuffer[i + 6] = v3x;
  e.renderVertexBuffer[i + 7] = v3y;
  e.renderVertexBuffer[i + 8] = v3z;

  calculateTriangleNormal(vertexCount, 3);

  i = vertexCount * VALUES_PER_COLOR;
  int j = 0;
  for (j = 0; j < 12; j += 1) {
    e.renderColorBuffer[i + j] = e.currentColor[j];
  }

  i = e.indexCount;
  for (int j = 0; j < 3; ++j) {
    e.renderIndexBuffer[i + j] = vertexCount + j;
  }

  e.vertexCount += 3;
  e.indexCount += 3;
}

void texTriangle(
    int textureId,
    float v1x, float v1y, float v1z, float u1, float v1,
    float v2x, float v2y, float v2z, float u2, float v2,
    float v3x, float v3y, float v3z, float u3, float v3) {
  setTexture(textureId);

  float *normals = e.renderNormalBuffer;

  int vertexCount = e.vertexCount;
  int indexCount = e.indexCount;

  int i = vertexCount * VALUES_PER_VERTEX;
  e.renderVertexBuffer[i] = v1x;
  e.renderVertexBuffer[i + 1] = v1y;
  e.renderVertexBuffer[i + 2] = v1z;
  e.renderVertexBuffer[i + 3] = v2x;
  e.renderVertexBuffer[i + 4] = v2y;
  e.renderVertexBuffer[i + 5] = v2z;
  e.renderVertexBuffer[i + 6] = v3x;
  e.renderVertexBuffer[i + 7] = v3y;
  e.renderVertexBuffer[i + 8] = v3z;

  calculateTriangleNormal(vertexCount, 3);

  i = vertexCount * VALUES_PER_TEXCOORD;
  e.renderTexCoordsBuffer[i] = u1;
  e.renderTexCoordsBuffer[i + 1] = v1;
  e.renderTexCoordsBuffer[i + 2] = u2;
  e.renderTexCoordsBuffer[i + 3] = v2;
  e.renderTexCoordsBuffer[i + 4] = u3;
  e.renderTexCoordsBuffer[i + 5] = v3;

  for (int j = 0; j < 3; ++j) {
    e.renderIndexBuffer[indexCount + j] = vertexCount + j;
  }

  e.vertexCount += 3;
  e.indexCount += 3;
}

void quad(
    float v1x, float v1y, float v1z,
    float v2x, float v2y, float v2z,
    float v3x, float v3y, float v3z,
    float v4x, float v4y, float v4z) {
  //1,2,3
  triangle(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z);

  //3,4,1
  int vertexCount = e.vertexCount;

  int i = vertexCount * VALUES_PER_VERTEX;
  e.renderVertexBuffer[i] = v4x;
  e.renderVertexBuffer[i + 1] = v4y;
  e.renderVertexBuffer[i + 2] = v4z;

  // copy the 4th normal vertex
  for (int k = 0; k < 3; ++k) {
    e.renderNormalBuffer[i + k] = e.renderNormalBuffer[i - 1 * 3 + k];
  }

  i = vertexCount * VALUES_PER_COLOR;
  for (int j = 0; j < 4; ++j) {
    e.renderColorBuffer[i + j] = e.currentColor[12 + j];
  }

  i = e.indexCount;
  e.renderIndexBuffer[i] = vertexCount - 1;
  e.renderIndexBuffer[i + 1] = vertexCount;
  e.renderIndexBuffer[i + 2] = vertexCount - 3;

  e.vertexCount += 1;
  e.indexCount += 3;
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
  int vertexCount = e.vertexCount;

  int i = vertexCount * VALUES_PER_VERTEX;
  e.renderVertexBuffer[i] = v4x;
  e.renderVertexBuffer[i + 1] = v4y;
  e.renderVertexBuffer[i + 2] = v4z;

  // copy the 4th normal vertex
  for (int k = 0; k < 3; ++k) {
    e.renderNormalBuffer[i + k] = e.renderNormalBuffer[i - 1 * 3 + k];
  }

  i = vertexCount * VALUES_PER_TEXCOORD;
  e.renderTexCoordsBuffer[i] = u4;
  e.renderTexCoordsBuffer[i + 1] = v4;

  i = e.indexCount;
  e.renderIndexBuffer[i] = vertexCount - 1;
  e.renderIndexBuffer[i + 1] = vertexCount;
  e.renderIndexBuffer[i + 2] = vertexCount - 3;

  e.vertexCount += 1;
  e.indexCount += 3;
}

int align(int x, int by) {
  int rest = x % by;
  return rest > 0
             ? x + by - rest
             : x;
}

int rgba(int r, int g, int b, int a) {
  return (a << 24) | (b << 16) | (g << 8) | r;
}

int rgb(int r, int g, int b) {
  return rgba(r, g, b, 255);
}

void rect(float x, float y, float z, float width, float height) {
  setTexture(-1);
  // quad(x, y, z, x, y + height, z, x + width, y + height, z, x + width, y, z);
  quad(
      x, y, z,
      x + width, y, z,
      x + width, y + height, z,
      x, y + height, z);
}

void texRect(int textureId, float x, float y, float z, float width, float height, float u1, float v1, float u0, float v0) {
  texQuad(
      textureId,
      x, y, z, u0, v0,
      x + width, y, z, u1, v0,
      x + width, y + height, z, u1, v1,
      x, y + height, z, u0, v1);
}

#ifndef PRODUCTION
void setTweakValue(int index, float value) {
  e.tweakValues[index] = value;
}
#endif