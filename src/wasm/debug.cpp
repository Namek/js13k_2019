#include "game.hpp"
#include "engine.hpp"

void debugRect(Transform &transform, Collider &collider, float z) {
  float
      x = transform.x - collider.width / 2,
      y = transform.y - collider.height / 2;
  rect(x, y, z, collider.width, collider.height);
}