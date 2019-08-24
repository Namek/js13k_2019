#include "models.hpp"
#include "../engine.hpp"
static int done = 0;
void renderModel3d(Model3d &model) {
  const uchar *vert = (uchar *)model.vertices;
  const float *vertf = (float *)model.verticesf;
  const uchar *triangles = (uchar *)model.faces3;
  const uchar *quads = (uchar *)model.faces4;

  const float s = model.scale;
  const float vAlignX = model.vertexAlignX;
  const float vAlignY = model.vertexAlignY;
  const float vAlignZ = model.vertexAlignZ;

  const int valuesPerVertex = 6;
  const int vx0 = 0;
  const int vx1 = 1;
  const int vy0 = 2;
  const int vy1 = 3;
  const int vz0 = 4;
  const int vz1 = 5;

  for (int i = 0; i < model.face3Count; i += 1) {
    const uint vi0 = (triangles[i * 3 + 0]) * valuesPerVertex;
    const uint vi1 = (triangles[i * 3 + 1]) * valuesPerVertex;
    const uint vi2 = (triangles[i * 3 + 2]) * valuesPerVertex;

    float v1x = ((float)vert[vi0 + vx0] + ((float)vert[vi0 + vx1] / 100.0f)) / s - vAlignX;
    float v1y = ((float)vert[vi0 + vy0] + ((float)vert[vi0 + vy1] / 100.0f)) / s - vAlignY;
    float v1z = ((float)vert[vi0 + vz0] + ((float)vert[vi0 + vz1] / 100.0f)) / s - vAlignZ;
    float v2x = ((float)vert[vi1 + vx0] + ((float)vert[vi1 + vx1] / 100.0f)) / s - vAlignX;
    float v2y = ((float)vert[vi1 + vy0] + ((float)vert[vi1 + vy1] / 100.0f)) / s - vAlignY;
    float v2z = ((float)vert[vi1 + vz0] + ((float)vert[vi1 + vz1] / 100.0f)) / s - vAlignZ;
    float v3x = ((float)vert[vi2 + vx0] + ((float)vert[vi2 + vx1] / 100.0f)) / s - vAlignX;
    float v3y = ((float)vert[vi2 + vy0] + ((float)vert[vi2 + vy1] / 100.0f)) / s - vAlignY;
    float v3z = ((float)vert[vi2 + vz0] + ((float)vert[vi2 + vz1] / 100.0f)) / s - vAlignZ;

    // v1x = vertf[vi0 / 2 + 0];
    // v1y = vertf[vi0 / 2 + 1];
    // v1z = vertf[vi0 / 2 + 2];
    // v2x = vertf[vi1 / 2 + 0];
    // v2y = vertf[vi1 / 2 + 1];
    // v2z = vertf[vi1 / 2 + 2];
    // v3x = vertf[vi2 / 2 + 0];
    // v3y = vertf[vi2 / 2 + 1];
    // v3z = vertf[vi2 / 2 + 2];

    triangle(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z);
  }
  done = 1;

  for (int i = 0; i < model.face4Count; i += 1) {
    const uint vi0 = quads[i * 4 + 0] * valuesPerVertex;
    const uint vi1 = quads[i * 4 + 1] * valuesPerVertex;
    const uint vi2 = quads[i * 4 + 2] * valuesPerVertex;
    const uint vi3 = quads[i * 4 + 3] * valuesPerVertex;

    float v1x = ((float)vert[vi0 + vx0] + ((float)vert[vi0 + vx1] / 100.0f)) / s - vAlignX;
    float v1y = ((float)vert[vi0 + vy0] + ((float)vert[vi0 + vy1] / 100.0f)) / s - vAlignY;
    float v1z = ((float)vert[vi0 + vz0] + ((float)vert[vi0 + vz1] / 100.0f)) / s - vAlignZ;
    float v2x = ((float)vert[vi1 + vx0] + ((float)vert[vi1 + vx1] / 100.0f)) / s - vAlignX;
    float v2y = ((float)vert[vi1 + vy0] + ((float)vert[vi1 + vy1] / 100.0f)) / s - vAlignY;
    float v2z = ((float)vert[vi1 + vz0] + ((float)vert[vi1 + vz1] / 100.0f)) / s - vAlignZ;
    float v3x = ((float)vert[vi2 + vx0] + ((float)vert[vi2 + vx1] / 100.0f)) / s - vAlignX;
    float v3y = ((float)vert[vi2 + vy0] + ((float)vert[vi2 + vy1] / 100.0f)) / s - vAlignY;
    float v3z = ((float)vert[vi2 + vz0] + ((float)vert[vi2 + vz1] / 100.0f)) / s - vAlignZ;
    float v4x = ((float)vert[vi3 + vx0] + ((float)vert[vi3 + vx1] / 100.0f)) / s - vAlignX;
    float v4y = ((float)vert[vi3 + vy0] + ((float)vert[vi3 + vy1] / 100.0f)) / s - vAlignY;
    float v4z = ((float)vert[vi3 + vz0] + ((float)vert[vi3 + vz1] / 100.0f)) / s - vAlignZ;

    // v1x = vertf[vi0 / 2 + 0];
    // v1y = vertf[vi0 / 2 + 1];
    // v1z = vertf[vi0 / 2 + 2];
    // v2x = vertf[vi1 / 2 + 0];
    // v2y = vertf[vi1 / 2 + 1];
    // v2z = vertf[vi1 / 2 + 2];
    // v3x = vertf[vi2 / 2 + 0];
    // v3y = vertf[vi2 / 2 + 1];
    // v3z = vertf[vi2 / 2 + 2];
    // v4x = vertf[vi3 / 2 + 0];
    // v4y = vertf[vi3 / 2 + 1];
    // v4z = vertf[vi3 / 2 + 2];

    quad(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, v4x, v4y, v4z);
  }
}