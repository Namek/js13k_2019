#ifndef ECS__H
#define ECS__H

#include "utils/array.hpp"

// ------------------
// Entity

struct Entity {
  unsigned int id;
  unsigned int componentBitSet;
};

// ------------------
// Component

#define DEF_COMPONENT(name) \
  struct name {             \
    static const unsigned int I = __COUNTER__;

#define END_COMPONENT \
  }                   \
  ;

#define A(componentType) (1 << (componentType::I))
#define SIZE(componentType) sizeof(componentType)

// ------------------
// EcsWorld

struct EcsWorld {
  Array entities;
  Array *components;
  int componentTypeCount;
  float deltaTime;


  Entity &newEntity() {
    int index;
    Entity &e = (Entity &)*entities.createPtrWithIndex(&index);
    e.id = index;
    e.componentBitSet = 0;
    return e;
  }

  void deleteEntity(int id) {
    Entity &e = (Entity &)*entities.getPtr(id);
    int bitset = e.componentBitSet;

    for (int i = 0; i < componentTypeCount; ++i) {
      if (((bitset << i) & 1) == 1) {
        components[i].remove(id);
      }
    }
    entities.remove(id);
  }

  void deleteEntity(Entity &e) {
    deleteEntity(e.id);
  }

  template <class T>
  T &hasComponent(int entityId) {
    constexpr int componentIndex = T::I;
    Entity &e = (Entity &)*entities.getPtr(entityId);
    bool hasComponent = (e.componentBitSet & (1 << componentIndex)) == 1;
    return hasComponent;
  }

  template <class T>
  T &getComponent(int entityId) {
    constexpr int componentIndex = T::I;
    return (T &)*components[componentIndex].getPtr(entityId);
  }

  // create or get
  template <class T>
  T &createComponent(int entityId) {
    constexpr int componentIndex = T::I;
    Entity &e = (Entity &)*entities.getPtr(entityId);

    if ((e.componentBitSet & (1 << componentIndex)) == 0) {
      e.componentBitSet |= (1 << componentIndex);

      return (T &)*components[componentIndex].createAtPtr(entityId);
    }
    else {
      return (T &)*components[componentIndex].getPtr(entityId);
    }
  }

  template <class T>
  void removeComponent(int entityId) {
    constexpr int componentIndex = T::I;
    Entity &e = (Entity &)*entities.getPtr(entityId);

    if ((e.componentBitSet & (1 << componentIndex)) == 1) {
      e.componentBitSet &= ~(1 << componentIndex);
      components[componentIndex].remove(entityId);
    }
    else {
      // TODO abort() if does not exist?
    }
  }

  Entity *getFirstEntityByAspect(int aspectIncludes) {
    for (int ie = ie, ne = entities.size; ie < ne; ++ie) {
      Entity *entity = (Entity *)entities.getPtr(ie);
      if ((entity->componentBitSet & aspectIncludes) != 0) {
        return entity;
      }
    }
    return 0;
  }
};

void initEcsWorld(EcsWorld &world, int componentTypeSizes[]);

// ------------------
// Entity System

#define DECL_ENTITY_SYTEM(name) \
  void name(EcsWorld &world);

#define DEF_ENTITY_SYSTEM(name, aspectIncludes)                  \
  void name(EcsWorld &world) {                                   \
    const int _aspectIncludes = aspectIncludes;                  \
    for (int ie = ie, ne = world.entities.size; ie < ne; ++ie) { \
      Entity &entity = (Entity &)*world.entities.getPtr(ie);     \
      if ((entity.componentBitSet & _aspectIncludes) != 0) {

#define END_ENTITY_SYSTEM \
  }                       \
  }                       \
  }

// ------------------
// Filtering entities by aspect

#define FOR_EACH_ENTITY(world, aspectIncludes)                 \
  for (int ie = ie, ne = world.entities.size; ie < ne; ++ie) { \
    Entity &entity = (Entity &)*world.entities.getPtr(ie);     \
    if ((entity.componentBitSet & aspectIncludes) != 0) {

#define END_FOR_EACH \
  }                  \
  }                  \
  }

#endif
