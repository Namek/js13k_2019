#include "game.hpp"
#include "textures.hpp"

// returns numbers:
//  - number of vertices (same as number of colors)
//  - number of indices
void render(float deltaTime) {
  beginFrame();

  float width = getCanvasWidth();
  float height = getCanvasHeight();
  float h = height;
  float w = width;
  float w2 = w / 2.0;
  float h2 = h / 2.0;

  const float aspectRatio = width / height;
  const float fieldOfViewInDegrees = 90.0 * PI_180;
  setProjectionMatrix(fieldOfViewInDegrees, aspectRatio, 1.0, 2000.0);

  // bottom left point is 0,0; center is width/2,height/2
  const float cameraZ = height / 2.0;
  setViewMatrix(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      -w2, -h2, -cameraZ, 1.0);

  // background
  const float z = 0;
  setColor(I, O, O, O);

  setColors4(
      1.0,
      1.0, 0.0, 0.0,
      0.0, 1.0, 0.0,
      0.0, 0.0, 1.0,
      I, 0.5, 0.0);

  quad(
      O, O, z,
      O, h2, z,
      w2 / 2.0, h2, z,
      width, O, z);
  quad(
      O, O, z,
      O, h2, z,
      w2 / 2.0, h2, z,
      width, O, z);

  texTriangle(TEXTURE_CHECKERBOARD,
              w2, h, O, O, O,
              w2, h / 3, O, O, 1.5,
              w, h / 3, O, w / h, 1.5);

  setColors3(1.0,
             1.0, 0.0, 0.0,
             0.0, 1.0, 0.0,
             0.0, 0.0, 1.0);

  int i = 0;
  for (i = 0; i < 8; i += 1) {
    const float m = i * 90;
    triangle(
        w2 - m, h2, 0.0,
        w2 - m, O, 0.0,
        w - m, O, 0.0);
  }

  endFrame();
}
