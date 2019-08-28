#ifndef MODELS__H
#define MODELS__H

#include "../common.hpp"
#include "../math/common.h"

struct Model3d {
  Vec3 vertexAlign;
  void *verticesCompressed;
  float scale;
  float *vertices;
  float *faceNormals;
  uchar *faces3;
  uchar *faces4;
  ushort vertexCount;
  ushort face3Count;
  ushort face4Count;
};

Model3d &getModel_frog();
void renderModel3d(Model3d &model);

float *renderFrog(float translation[VEC_SIZE_3], float orientation[MAT_SIZE_4]);

#endif