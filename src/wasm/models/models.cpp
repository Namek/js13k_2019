#include "models.hpp"
#include "../engine.hpp"
static int done = 0;
void renderModel3d(Model3d &model) {
  const uchar *vert = (uchar *)model.vertices;
  const uchar *triangles = (uchar *)model.faces3;
  const uchar *quads = (uchar *)model.faces4;

  const float s = model.scale;
  const float vAlignX = s * model.vertexAlignX;
  const float vAlignY = s * model.vertexAlignY;
  const float vAlignZ = s * model.vertexAlignZ;

  const int valuesPerVertex = 6;
  const int vx0 = 0;
  const int vx1 = 1;
  const int vy0 = 2;
  const int vy1 = 3;
  const int vz0 = 4;
  const int vz1 = 5;

  for (int i = 0; i < model.face3Count; i += 1) {
    const uint vi0 = triangles[i * 3 + 0] * valuesPerVertex;
    const uint vi1 = triangles[i * 3 + 1] * valuesPerVertex;
    const uint vi2 = triangles[i * 3 + 2] * valuesPerVertex;

    float v1x = ((float)vert[vi0 + vx0] + ((float)vert[vi0 + vx1] / 100.0f)) - vAlignX;
    float v1y = ((float)vert[vi0 + vy0] + ((float)vert[vi0 + vy1] / 100.0f)) - vAlignY;
    float v1z = ((float)vert[vi0 + vz0] + ((float)vert[vi0 + vz1] / 100.0f)) - vAlignZ;
    float v2x = ((float)vert[vi1 + vx0] + ((float)vert[vi1 + vx1] / 100.0f)) - vAlignX;
    float v2y = ((float)vert[vi1 + vy0] + ((float)vert[vi1 + vy1] / 100.0f)) - vAlignY;
    float v2z = ((float)vert[vi1 + vz0] + ((float)vert[vi1 + vz1] / 100.0f)) - vAlignZ;
    float v3x = ((float)vert[vi2 + vx0] + ((float)vert[vi2 + vx1] / 100.0f)) - vAlignX;
    float v3y = ((float)vert[vi2 + vy0] + ((float)vert[vi2 + vy1] / 100.0f)) - vAlignY;
    float v3z = ((float)vert[vi2 + vz0] + ((float)vert[vi2 + vz1] / 100.0f)) - vAlignZ;

    if (!done && i == model.face3Count - 1) {
      uchar c = vert[0];
      _l((int)v1x);
      _l(v1y);
      _l(v1z);
      _l(v2x);
      _l(v2y);
      _l(v2z);
      _l(v3x);
      _l(v3y);
      _l(v3z);
    }

    triangle(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z);
  }
  done = 1;

  // TODO all of this is broken!


  for (int i = 0; i < model.face4Count; i += 1) {
    const uint vi0 = quads[i * 4 + 0] * valuesPerVertex;
    const uint vi1 = quads[i * 4 + 1] * valuesPerVertex;
    const uint vi2 = quads[i * 4 + 2] * valuesPerVertex;
    const uint vi3 = quads[i * 4 + 3] * valuesPerVertex;

    float v1x = ((float)vert[vi0 + vx0] + ((float)vert[vi0 + vx1] / 100.0f)) - vAlignX;
    float v1y = ((float)vert[vi0 + vx0] + ((float)vert[vi0 + vy1] / 100.0f)) - vAlignY;
    float v1z = ((float)vert[vi0 + vz0] + ((float)vert[vi0 + vz1] / 100.0f)) - vAlignZ;
    float v2x = ((float)vert[vi1 + vx0] + ((float)vert[vi1 + vx1] / 100.0f)) - vAlignX;
    float v2y = ((float)vert[vi1 + vy0] + ((float)vert[vi1 + vy1] / 100.0f)) - vAlignY;
    float v2z = ((float)vert[vi1 + vz0] + ((float)vert[vi1 + vz1] / 100.0f)) - vAlignZ;
    float v3x = ((float)vert[vi2 + vx0] + ((float)vert[vi2 + vx1] / 100.0f)) - vAlignX;
    float v3y = ((float)vert[vi2 + vy0] + ((float)vert[vi2 + vy1] / 100.0f)) - vAlignY;
    float v3z = ((float)vert[vi2 + vz0] + ((float)vert[vi2 + vz1] / 100.0f)) - vAlignZ;
    float v4x = ((float)vert[vi3 + vx0] + ((float)vert[vi3 + vx1] / 100.0f)) - vAlignX;
    float v4y = ((float)vert[vi3 + vy0] + ((float)vert[vi3 + vy1] / 100.0f)) - vAlignY;
    float v4z = ((float)vert[vi3 + vz0] + ((float)vert[vi3 + vz1] / 100.0f)) - vAlignZ;

    // quad(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, v4x, v4y, v4z);
  }
}