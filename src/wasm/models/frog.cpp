#include "models.hpp"
#include "../engine.hpp"
// clang-format off

Model3d &getModel_frog() {
  static unsigned char vertices[] = {
    0, 0, 0, 0, 59, 59, 20, 93, 22, 18, 47, 81,  //
    11, 72, 0, 1, 44, 48, 87, 7, 18, 14, 15, 95,  //
    50, 27, 0, 5, 26, 80, 251, 39, 0, 13, 14, 41,  // 
    251, 2, 0, 26, 14, 25, 223, 95, 14, 59, 8, 28,  // 
    216, 41, 0, 22, 4, 78, 205, 44, 0, 21, 5, 92,  // 
    118, 56, 38, 25, 19, 55, 133, 94, 19, 26, 20, 84,  // 
    236, 73, 0, 24, 8, 10, 207, 85, 13, 42, 4, 54,  // 
    205, 25, 25, 81, 44, 21, 238, 2, 31, 74, 23, 3,  // 
    163, 34, 30, 87, 58, 33, 89, 9, 47, 33, 69, 73,  // 
    130, 67, 46, 58, 60, 80, 160, 99, 50, 18, 51, 31,  // 
    252, 95, 12, 82, 21, 52, 254, 88, 0, 26, 20, 69,  // 
    255, 0, 0, 13, 20, 63, 187, 41, 0, 19, 49, 94,  // 
    210, 0, 0, 22, 42, 57, 163, 34, 0, 17, 58, 33,  // 
    240, 82, 0, 25, 28, 20, 89, 9, 0, 9, 69, 73,  // 
    2, 23, 0, 0, 61, 29, 10, 38, 14, 99, 65, 29,  // 
    121, 11, 84, 43, 79, 12, 130, 77, 77, 56, 79, 12,  // 
    114, 19, 86, 60, 65, 64, 141, 36, 99, 62, 79, 13,  // 
    222, 89, 18, 21, 12, 12, 220, 83, 25, 63, 15, 6,  // 
    246, 81, 11, 22, 12, 70, 90, 15, 40, 16, 24, 77,  //
    127, 87, 49, 35, 42, 68, 149, 38, 75, 26, 75, 53,  // 
    171, 45, 77, 7, 77, 53, 24, 82, 18, 16, 71, 6,  // 
    159, 23, 49, 60, 34, 93, 196, 18, 50, 23, 36, 48,  // 
    147, 92, 41, 76, 25, 35, 205, 46, 29, 83, 11, 12,  // 
    205, 69, 25, 5, 3, 14, 64, 36, 39, 46, 45, 66,  // 
    214, 17, 29, 34, 17, 20, 102, 12, 53, 39, 52, 71,  // 
    154, 29, 97, 45, 77, 20, 149, 5, 78, 96, 62, 47,  // 
    131, 10, 84, 90, 66, 16, 218, 22, 16, 73, 1, 36,  // 
    210, 62, 19, 90, 0, 0, 6, 87, 21, 20, 79, 12,  // 
    24, 32, 21, 86, 79, 12, 32, 52, 95, 99, 79, 13,  // 
    9, 13, 24, 54, 67, 36, 59, 1, 66, 51, 79, 12,  // 
    71, 47, 97, 2, 68, 9, 36, 80, 94, 1, 69, 28,  // 
    114, 17, 117, 50, 79, 12, 112, 72, 112, 27, 68, 70,  // 
    61, 48, 55, 4, 39, 78, 110, 31, 85, 94, 51, 65,  // 
    96, 77, 84, 98, 66, 97, 83, 59, 104, 9, 49, 55,  // 
    41, 34, 75, 52, 56, 2, 40, 81, 79, 37, 70, 1,  // 
    89, 25, 81, 83, 37, 81, 133, 94, 0, 14, 20, 84,  // 
    87, 7, 0, 9, 15, 95, 24, 82, 0, 2, 71, 6,  // 
    175, 8, 0, 18, 11, 72, 221, 66, 23, 17, 5, 90,  // 
    217, 59, 26, 52, 6, 46, 220, 57, 26, 87, 10, 15,  // 
    213, 38, 26, 67, 2, 23, 212, 6, 29, 67, 7, 78,  // 
    214, 96, 30, 9, 11, 52, 218, 70, 23, 12, 2, 62, 
  };
  static unsigned char faces3[] = {
    2, 3, 4, 36, 5, 6, 7, 36, 13, 8, 9, 13,  // 
    10, 44, 11, 10, 11, 3, 6, 12, 36, 20, 15, 14,  // 
    43, 14, 15, 17, 16, 18, 19, 18, 16, 20, 21, 22,  // 
    23, 24, 14, 24, 26, 14, 26, 21, 14, 28, 29, 0,  // 
    49, 17, 18, 49, 18, 38, 30, 31, 32, 30, 32, 33,  // 
    7, 34, 36, 19, 51, 39, 51, 40, 39, 38, 44, 10,  // 
    42, 43, 44, 13, 45, 46, 29, 47, 1, 37, 10, 3,  // 
    15, 20, 36, 42, 19, 43, 35, 48, 15, 45, 44, 43,  // 
    45, 43, 15, 45, 15, 48, 52, 44, 38, 42, 51, 19,  // 
    52, 50, 51, 40, 51, 50, 52, 38, 18, 37, 1, 47,  // 
    37, 47, 49, 39, 50, 52, 39, 52, 18, 39, 18, 19,  // 
    7, 13, 53, 13, 46, 54, 53, 13, 54, 56, 57, 55,  // 
    59, 41, 60, 56, 41, 59, 60, 41, 66, 66, 41, 17,  // 
    32, 63, 33, 33, 63, 62, 64, 47, 29, 65, 70, 67,  // 
    58, 68, 29, 68, 58, 69, 69, 58, 61, 61, 60, 69,  // 
    67, 69, 60, 64, 29, 68, 70, 65, 49, 67, 68, 69,  // 
    31, 59, 32, 63, 60, 61, 59, 60, 32, 55, 58, 29,  // 
    33, 57, 59, 33, 59, 30, 33, 62, 57, 40, 50, 39,  // 
    63, 32, 60, 41, 29, 28, 41, 28, 73, 20, 14, 21,  // 
    11, 13, 74, 11, 74, 71, 13, 9, 74, 13, 36, 12,  // 
    13, 12, 8, 3, 72, 4, 78, 79, 76, 81, 78, 76,  // 
    48, 80, 45, 45, 80, 79, 54, 46, 78, 75, 34, 7, 
  };
  static unsigned char faces4[] = {
    1, 37, 3, 2, 81, 76, 77, 75, 29, 41, 56, 55, 80, 77, 76, 79,  // 
    35, 77, 80, 48, 1, 2, 0, 29, 16, 17, 27, 25, 49, 65, 66, 17,  // 
    73, 27, 17, 41, 11, 44, 45, 13, 55, 57, 61, 58, 51, 42, 44, 52,  // 
    79, 78, 46, 45, 81, 53, 54, 78, 15, 36, 34, 35, 10, 37, 49, 38,  // 
    20, 22, 5, 36, 67, 70, 64, 68, 67, 60, 66, 65, 53, 81, 75, 7,  //
    62, 63, 61, 57, 35, 34, 75, 77, 49, 47, 64, 70, 14, 16, 25, 23,  // 
    19, 16, 14, 43, 3, 11, 71, 72, 
  };
  const ushort vertexCount = 82;
  const ushort face3Count = 88;
  const ushort face4Count = 26;

  const float vertexAlignX = 128.7685;
  const float vertexAlignY = 0.144895;
  const float vertexAlignZ = 85.68532;
  const float scale = 0.9235123;


  static Model3d model;
  model.vertexAlignX = vertexAlignX;
  model.vertexAlignY = vertexAlignY;
  model.vertexAlignZ = vertexAlignZ;
  model.scale = scale;
  model.vertices = vertices;
  model.faces3 = faces3;
  model.faces4 = faces4;
  model.vertexCount = vertexCount;
  model.face3Count = face3Count;
  model.face4Count = face4Count;

  return model;
}

// returns frog's matrix. The matrix is already push()ed, remember to pop().
float* renderFrog(float x, float y, float z) {
  const float frogScale = 0.2;
  const float tZ = 85.7 * frogScale;

  auto mat = mat4_identity(pushModelMatrix());
  mat4_translate(mat, mat, vec3_set(vec3Tmp, x, y, z));
  mat4_rotateX(mat, mat, toRadian(-180));
  mat4_rotateY(mat, mat, toRadian(0));
  mat4_rotateZ(mat, mat, toRadian(90));
  // mat4_translate(mat, mat, vec3_set(vec3Tmp, 0, 0, tZ));
  mat4_scale(mat, mat, vec3_set(vec3Tmp, frogScale, frogScale, frogScale));
  ref model = getModel_frog();

  setColor(1, 182/255, 1, 109/255);
  renderModel3d(model);
  flushBuffers();

  // other half of the frog
  mat4_scale(mat, mat, vec3_set(vec3Tmp, 1, -1, 1));
  renderModel3d(model);

  popModelMatrix();

  return mat;
}