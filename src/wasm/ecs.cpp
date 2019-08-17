#include "ecs.hpp"

void initEcsWorld(EcsWorld &world, int componentTypeSizes[]) {
  world.entities.init(sizeof(Entity));

  int componentTypeCount = sizeof(*componentTypeSizes) / sizeof(int);
  world.componentTypeCount = componentTypeCount;
  world.components = new Array[componentTypeCount];
  for (int i = 0; i < componentTypeCount; ++i)
    world.components[i].init(componentTypeSizes[i]);
}