#include "game.hpp"
#include "engine.hpp"

void debugRect(Transform &transform, Collider &collider) {
  float
      x = transform.pos.x - collider.width / 2,
      y = transform.pos.y - collider.height / 2;
  rect(x, y, transform.pos.vec[2], collider.width, collider.height);
}