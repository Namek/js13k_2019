#include "ecs.hpp"

// ------------
// EntitySystem

template <class TCmp>
TCmp &IEntitySystem::getCmp(Entity &entity) {
  constexpr int i = TCmp::I;
  Component &c = world->components[i][entity.id];
  return (TCmp &)c;
}

template <class TCmp>
TCmp &IEntitySystem::getCmp(int entityId) {
  constexpr int i = TCmp::I;
  Component &c = world->components[i][entityId];
  return (TCmp &)c;
}

EntityEdit &IEntitySystem::edit(Entity &entity) {
  return world->edit(entity);
}


// -----------
// EcsWorld

EcsWorld::EcsWorld(int componentTypesCount, IEntitySystem systems[]) {
  reusableEntityEdit->world = this;

  this->componentTypesCount = componentTypesCount;
  components = new Array<Component>[componentTypesCount];

  unsigned int systemCount = sizeof(*systems) / sizeof(IEntitySystem *);
  for (unsigned int i = 0; i < systemCount; ++i)
    this->systems.add(&systems[i]);
}

void EcsWorld::process() {
  for (int is = 0, ns = systems.size; is < ns; ++is) {
    auto system = systems[is];
    int systemAspect = system->aspectIncludes;

    for (int ie = ie, ne = entities.size; ie < ne; ++ie) {
      Entity &entity = entities[ie];
      if ((entity.componentBitSet & systemAspect) != 0) {
        system->processEntity(entity);
      }
    }
  }
}

EntityEdit &EcsWorld::newEntity() {
  return edit(entities.create());
}

EntityEdit &EcsWorld::edit(Entity &entity) {
  reusableEntityEdit->entity = &entity;
  return *reusableEntityEdit;
}

// ---------
// EntityEdit

template <unsigned int INDEX>
void EntityEdit::set(IndexedComponent<INDEX> &c) {
  constexpr int i = INDEX;
  world->components[i].set(entity->id, c);
}

EntityEdit &Entity::edit(EcsWorld &world) {
  return world.edit(*this);
}

// template <
//     template <unsigned int> class TComponent, unsigned int INDEX>
// TComponent<INDEX> &EntityEdit::get() {
//   constexpr int componentIndex = INDEX;
//   IndexedComponent<INDEX> &c = world->components[componentIndex][entity->id];
//   return c;
// }