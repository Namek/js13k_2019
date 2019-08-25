#include "textures.hpp"

extern int OFFSET_FUNC_RETURN;
extern int SIZE_FUNC_RETURN;

struct TextureInfo {
  int width;
  int height;
};

TextureInfo texInfo;

void checkerboard(int *tex);
void grass(int *tex);

#define SEND_TEXTURE(fn) \
  fn(tex);                   \
  sendTexture((int)tex, texInfo.width, texInfo.height);

// call it before initEngine()
void generateTextures() {
  int textureStart = align(OFFSET_FUNC_RETURN + SIZE_FUNC_RETURN + 1 + 2, 4); //make some space for info about textures
  int *tex = (int *)textureStart;

  SEND_TEXTURE(checkerboard)
  SEND_TEXTURE(grass)
}

void checkerboard(int *tex) {
  const int w = 500;
  const int h = 500;
  const int gap = 50;
  int color = 0;

  // checkerboard: TEXTURE_CHECKERBOARD
  for (int y = 0; y < h; y += gap) {
    for (int x = 0; x < w; x += gap) {
      // fill rectangle
      for (int i = 0; i < gap; i += 1) {
        for (int j = 0; j < gap; j += 1) {
          int p = (y + i) * w + x + j;
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

  for (int y = 0; y < h; y += 1) {
    for (int x = 0; x < w; x += 1) {
      int p = y * w + x;
      float v = 128.0f + (40.0f * randomf());
      tex[p] = rgb(0, CLAMP255(v), 0);
    }
  }

  texInfo.width = w;
  texInfo.height = h;
}
