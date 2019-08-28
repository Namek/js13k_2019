#include "models.hpp"
#include "../engine.hpp"

// #define DEBUG_NORMALS

void decompressVerticesAndcalculateTriangleNormals(Model3d &model) {
  const uchar *vert = (uchar *)model.verticesCompressed;

  for (int fsize = 3, fiTotal = 0; fsize <= 4; ++fsize) {
    const uchar *faces = (uchar *)(fsize == 3 ? model.faces3 : model.faces4);
    const uint faceCount = fsize == 3 ? model.face3Count : model.face4Count;

    // TODO precalculate normals, right now we're making them on the fly by calling calculateTriangleNormal() below
  }
}

void renderModel3d(Model3d &model) {
  const uchar *vert = (uchar *)model.verticesCompressed;
  const float *faceNormals = model.faceNormals;
  const float s = model.scale;
  const int valuesPerVertex = 6;

  static float faceVertices[4 * 3];
  static float faceNormal[3];

#ifndef PRODUCTION
  static float curColor[4];
  for (uint i = 0; i < 4; ++i)
    curColor[i] = engineState.currentColor[i];
#endif

  for (int fsize = 3, fiTotal = 0; fsize <= 4; ++fsize) {
    const uchar *faces = (uchar *)(fsize == 3 ? model.faces3 : model.faces4);
    const uint faceCount = fsize == 3 ? model.face3Count : model.face4Count;

    for (int fi = 0; fi < faceCount; fi += 1, fiTotal += 1) {
      // collect normal vertex + 3/4 vertices
      for (int ci = 0; ci < fsize; ++ci) {
        const uint vi = faces[fi * fsize + ci] * valuesPerVertex;

        for (int k = 0; k < 3; ++k) {
          faceVertices[ci * 3 + k] = ((float)vert[vi + k * 2 + 0] + ((float)vert[vi + k * 2 + 1] / 100.0f)) / s - model.vertexAlign.vec[k];
        }
      }

      int vertexIndex;
      for (int k = 0; k < fsize * 3; k += 3) {
        vertexIndex = vertex(faceVertices[k], faceVertices[k + 1], faceVertices[k + 2]);
      }

      if (fsize == 3) {
        calculateTriangleNormal(vertexIndex - 2, fsize);
        index(vertexIndex - 2);
        index(vertexIndex - 1);
        index(vertexIndex);
      }
      else if (fsize == 4) {
        calculateTriangleNormal(vertexIndex - 3, fsize);
        index(vertexIndex - 3);
        index(vertexIndex - 2);
        index(vertexIndex - 1);
        index(vertexIndex - 1);
        index(vertexIndex);
        index(vertexIndex - 3);
      }

#ifndef PRODUCTION
#ifdef DEBUG_NORMALS
      // debug: draw lines for vertex normals
      setColor(1, 1, 182 / 255.0f, 193 / 255.0f);
      Vec3 center;
      vec3_set(center.vec, 0, 0, 0);
      for (int i = 0; i < fsize; ++i) {
        vec3_add(center.vec, center.vec, faceVertices + 3 * i);
      }
      vec3_scale(center.vec, center.vec, 1.0f / (float)fsize);
      const float l = 15.0f;
      const float w = 1.3f;
      quad(
          center.x + u.x * l, center.y + u.y * l, center.z + u.z * l,
          center.x + u.x * l + w, center.y + u.y * l, center.z + u.z * l,
          center.x + w, center.y, center.z,
          center.x, center.y, center.z);

      for (uint i = 0; i < 4; ++i)
        engineState.currentColor[i] = curColor[i];
#endif
#endif
    }
  }
}