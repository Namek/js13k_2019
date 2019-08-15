#include "game.hpp"
#include "textures.hpp"

// returns numbers:
//  - number of vertices (same as number of colors)
//  - number of indices
void render(float deltaTime) {
  beginFrame();

  float canvasWidth = getCanvasWidth();
  float canvasHeight = getCanvasHeight();
  float bottomUiHeight = 100;
  float h = canvasHeight - bottomUiHeight;
  float w = canvasWidth;
  float w2 = w / 2.0;
  float h2 = h / 2.0;

  const float aspectRatio = canvasWidth / canvasHeight;
  const float fieldOfViewInDegrees = 90.0 * PI_180;
  setProjectionMatrix(fieldOfViewInDegrees, aspectRatio, 1.0, 2000.0);

  // bottom left point is 0,0; center is width/2,height/2
  const float cameraZ = canvasHeight / 2.0;
  setViewMatrix(
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      -canvasWidth/2, -canvasHeight/2 + bottomUiHeight, -cameraZ, 1.0);

  // background
  const float z = 0;
  // setColor(I, O, O, O);

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
      w, O, z);
  quad(
      O, O, z,
      O, h2, z,
      w2 / 2.0, h2, z,
      w, O, z);

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

  // layout here is:
  // - grass
  // - roadside
  // - road:
  //   - (lane, gap)...
  //   - lane
  // - roadside
  // - grass

  float laneHeight = 70;
  int laneCount = 4;
  float lanesGap = 4;
  float roadsideHeight = 40;
  float roadHeight = laneCount * laneHeight + ((laneCount-1) * lanesGap);

  float grassHeight = (h - roadHeight - 2*roadsideHeight)/2;

  // grass - top
  setColors4(1,
    0, 1, 0.04,
    0, 1, 0.04,
    0, 0.7, 0.04,
    0, 0.7, 0.04
  );
  quad(
    0, 0, z,
    0, grassHeight, z,
    w, grassHeight, z,
    w, 0, z
  );

  // grass - bottom
  setColors4(1,
    0, 1, 0.04,
    0, 0.7, 0.04,
    0, 0.7, 0.04,
    0, 1, 0.04
  );
  quad(
    0, h, z,
    w, h, z,
    w, h-grassHeight, z,
    0, h-grassHeight, z
  );

  float roadY = grassHeight + roadsideHeight;

  // roadsides
  setColorLeftToRight(1, 0.6, 0.6, 0.6, 0.5, 0.5, 0.5);
  rect(0, roadY - roadsideHeight, z, w, roadsideHeight);
  rect(0, roadY + roadHeight, z, w, roadsideHeight);

  // black road
  setColorLeftToRight(1, 0.1, 0.1, 0.1, 0, 0, 0);
  rect(0, roadY, z, w, roadHeight);

  // draw lanes from top to down
  float laneY = roadY + roadHeight;
  for (int i = 0; i < laneCount-1; ++i) {
    laneY -= (laneHeight+lanesGap);

    setColor(1, 1, 1, 1);
    rect(0, laneY, z, w, lanesGap);
  }

  endFrame();
}
