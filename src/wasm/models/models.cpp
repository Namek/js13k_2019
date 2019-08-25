#include "models.hpp"
#include "../engine.hpp"

void renderModel3d(Model3d &model) {
  const uchar *vert = (uchar *)model.vertices;
  const float s = model.scale;
  const int valuesPerVertex = 6;

  static float vAlign[3];
  vAlign[0] = model.vertexAlignX;
  vAlign[1] = model.vertexAlignY;
  vAlign[2] = model.vertexAlignZ;

  static float faceVertices[5 * 3];

  for (int fsize = 3; fsize <= 4; ++fsize) {
    const uint faceSize = fsize + 1; //3 vertices + normal
    const uchar *faces = (uchar *)(fsize == 3 ? model.faces3 : model.faces4);
    const uint faceCount = fsize == 3 ? model.face3Count : model.face4Count;

    for (int fi = 0; fi < faceCount; fi += 1) {

      // collect normal vertex + 3/4 vertices
      for (int ci = 0; ci < faceSize; ++ci) {
        const uint vi = faces[fi * faceSize + ci] * valuesPerVertex;

        for (int k = 0; k < 3; ++k) {
          faceVertices[ci * 3 + k] = ((float)vert[vi + k * 2 + 0] + ((float)vert[vi + k * 2 + 1] / 100.0f)) / s - vAlign[k];
        }
      }

      setCurrentVertexNormal(faceVertices[0], faceVertices[1], faceVertices[2]);
      int vertexIndex;
      for (int k = 3; k < faceSize * 3; k += 3) {
        vertexIndex = vertex(faceVertices[k], faceVertices[k + 1], faceVertices[k + 2]);
      }

      if (fsize == 3) {
        index(vertexIndex - 2);
        index(vertexIndex - 1);
        index(vertexIndex);
      }
      else if (fsize == 4) {
        index(vertexIndex - 3);
        index(vertexIndex - 2);
        index(vertexIndex - 1);
        index(vertexIndex - 1);
        index(vertexIndex);
        index(vertexIndex - 3);
      }
    }
  }
}