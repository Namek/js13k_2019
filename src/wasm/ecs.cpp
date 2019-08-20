#include "ecs.hpp"

void initEcsWorld(EcsWorld &world, int componentTypeSizes[], int componentTypeCount) {
  world.deltaTime = 0;
  world.entities.init(sizeof(Entity));
  world.componentTypeCount = componentTypeCount;
  world.components = (Array *)malloc(sizeof(Array) * componentTypeCount);

  for (int i = 0; i < componentTypeCount; ++i) {
    world.components[i].init(componentTypeSizes[i]);
  }
}