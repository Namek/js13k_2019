#include "models.hpp"
#include "../engine.hpp"
static int done = 0;
void renderModel3d(Model3d &model) {
  uchar *vert = (uchar *)model.vertices;
  uchar *triangles = (uchar *)model.faces3;
  uchar *quads = (uchar *)model.faces4;

  for (int it = 0; it < model.face3Count; it += 3) {
    const int v1i = triangles[it * 3];
    const int v2i = triangles[it * 3 + 1];
    const int v3i = triangles[it * 3 + 2];

    float v1x = (float)vert[triangles[it * 3 + 0] * 6 + 0] + ((float)vert[triangles[it * 3 + 0] * 6 + 1] / 100.0f); // + model.vertexAlignX;
    float v1y = (float)vert[triangles[it * 3 + 0] * 6 + 2] + ((float)vert[triangles[it * 3 + 0] * 6 + 3] / 100.0f); // + model.vertexAlignY;
    float v1z = (float)vert[triangles[it * 3 + 0] * 6 + 4] + ((float)vert[triangles[it * 3 + 0] * 6 + 5] / 100.0f); // + model.vertexAlignZ;
    float v2x = (float)vert[triangles[it * 3 + 1] * 6 + 0] + ((float)vert[triangles[it * 3 + 1] * 6 + 1] / 100.0f); // + model.vertexAlignX;
    float v2y = (float)vert[triangles[it * 3 + 1] * 6 + 2] + ((float)vert[triangles[it * 3 + 1] * 6 + 3] / 100.0f); // + model.vertexAlignY;
    float v2z = (float)vert[triangles[it * 3 + 1] * 6 + 4] + ((float)vert[triangles[it * 3 + 1] * 6 + 5] / 100.0f); // + model.vertexAlignZ;
    float v3x = (float)vert[triangles[it * 3 + 2] * 6 + 0] + ((float)vert[triangles[it * 3 + 2] * 6 + 1] / 100.0f); // + model.vertexAlignX;
    float v3y = (float)vert[triangles[it * 3 + 2] * 6 + 2] + ((float)vert[triangles[it * 3 + 2] * 6 + 3] / 100.0f); // + model.vertexAlignY;
    float v3z = (float)vert[triangles[it * 3 + 2] * 6 + 4] + ((float)vert[triangles[it * 3 + 2] * 6 + 5] / 100.0f); // + model.vertexAlignZ;

    if (!done && it == 0) {
      done = 1;
      uchar c = vert[0];
      _l((int)c);
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

  // TODO all of this is broken!

  // vert[()*2], vert[(quads[it*4+0])*2+1]

  for (int it = 0; it < model.face4Count; it += 4) {
    float v1x = (float)vert[quads[it * 4 + 0] * 6+0] + ((float)vert[quads[it * 4 + 0] * 6 + 1] / 100.0f);
    float v1y = (float)vert[quads[it * 4 + 0] * 6+2] + ((float)vert[quads[it * 4 + 0] * 6 + 3] / 100.0f);
    float v1z = (float)vert[quads[it * 4 + 0] * 6+4] + ((float)vert[quads[it * 4 + 0] * 6 + 5] / 100.0f);
    float v2x = (float)vert[quads[it * 4 + 1] * 6+0] + ((float)vert[quads[it * 4 + 1] * 6 + 1] / 100.0f);
    float v2y = (float)vert[quads[it * 4 + 1] * 6+2] + ((float)vert[quads[it * 4 + 1] * 6 + 3] / 100.0f);
    float v2z = (float)vert[quads[it * 4 + 1] * 6+4] + ((float)vert[quads[it * 4 + 1] * 6 + 5] / 100.0f);
    float v3x = (float)vert[quads[it * 4 + 2] * 6+0] + ((float)vert[quads[it * 4 + 2] * 6 + 1] / 100.0f);
    float v3y = (float)vert[quads[it * 4 + 2] * 6+2] + ((float)vert[quads[it * 4 + 2] * 6 + 3] / 100.0f);
    float v3z = (float)vert[quads[it * 4 + 2] * 6+4] + ((float)vert[quads[it * 4 + 2] * 6 + 5] / 100.0f);
    float v4x = (float)vert[quads[it * 4 + 3] * 6+0] + ((float)vert[quads[it * 4 + 3] * 6 + 1] / 100.0f);
    float v4y = (float)vert[quads[it * 4 + 3] * 6+2] + ((float)vert[quads[it * 4 + 3] * 6 + 3] / 100.0f);
    float v4z = (float)vert[quads[it * 4 + 3] * 6+4] + ((float)vert[quads[it * 4 + 3] * 6 + 5] / 100.0f);

    quad(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, v4x, v4y, v4z);
  }
}