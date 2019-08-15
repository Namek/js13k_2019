#include "textures.hpp"

extern int OFFSET_FUNC_RETURN;
extern int SIZE_FUNC_RETURN;

struct TextureInfo {
  int width;
  int height;
};

TextureInfo texInfo;
int x = 0;
int y = 0;
int i = 0;
int j = 0;
int p = 0;

void checkerboard(int *tex);
void grass(int *tex);

#define GENERATE_TEXTURE(fn)                                    \
  fn(tex);                                                      \
  ret[ri++] = texInfo.width;                                    \
  ret[ri++] = texInfo.height;                                   \
  ret[ri++] = (int)tex;                                         \
  tex = (int *)((int)tex + texInfo.width * texInfo.height * 4); \
  textureCount += 1;


// call it before initEngine()
void generateTextures() {
  int *ret = (int *)OFFSET_FUNC_RETURN;

  int ALL_TEXTURES = 2; // TODO it would be better to emit textures one by one instead of creating them all at once

  int textureStart = align(OFFSET_FUNC_RETURN + SIZE_FUNC_RETURN + 1 + 3 * ALL_TEXTURES, 4); //make some space for info about textures
  int *tex = (int *)textureStart;

  int ri = 0, textureCount = 0;
  ret[ri++] = 0;

  GENERATE_TEXTURE(checkerboard)
  GENERATE_TEXTURE(grass)

  ret[0] = textureCount;

  l(ret[0]);
  l(ret[1]);
  l(ret[2]);
  l(ret[3]);
}

void checkerboard(int *tex) {
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

  texInfo.width = w;
  texInfo.height = h;
}

void grass(int *tex) {
  const int w = 500;
  const int h = 500;

  for (y = 0; y < h; y += 1) {
    for (x = 0; x < w; x += 1) {
      p = y * w + x;
      float v = 128.0f + (40.0f * random());
      tex[p] = rgb(0, CLAMP255(v), 0);
    }
  }

  texInfo.width = w;
  texInfo.height = h;
}
