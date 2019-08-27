#ifndef MODELS__H
#define MODELS__H

#include "../common.hpp"
#include "../math/common.h"

struct Model3d {
  float vertexAlignX;
  float vertexAlignY;
  float vertexAlignZ;
  float scale;
  void *vertices;
  void *verticesf;
  void *faces3;
  void *faces4;
  ushort vertexCount;
  ushort face3Count;
  ushort face4Count;
};

Model3d &getModel_frog();
Model3d &getModel_test();
void renderModel3d(Model3d &model);

float* renderFrog(float translation[VEC_SIZE_3]);

#endif