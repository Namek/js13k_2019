#include "textures.hpp"

extern int OFFSET_FUNC_RETURN;
extern int SIZE_FUNC_RETURN;

// call it before initEngine()
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