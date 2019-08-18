#ifndef ECS__H
#define ECS__H

#include "utils/array.hpp"
#include <stdarg.h> // for va_list

extern void l(int);

struct Component {};

template <unsigned int INDEX>
struct IndexedComponent : Component {};

#define DEF_COMPONENT(name)                           \
  struct name : IndexedComponent<(__COUNTER__ / 2)> { \
    static const unsigned int I = __COUNTER__ / 2;

#define DEF_END \
  }             \
  ;

class EcsWorld;
struct EntityEdit;

struct Entity {
  unsigned int id;
  unsigned int componentBitSet;

  EntityEdit &edit(EcsWorld &world);
};

// template <>
struct AAComponentMapper {

  template <unsigned int INDEX>
  void add(Entity &entity, IndexedComponent<INDEX> &c) {
    constexpr int i = INDEX;

    entity.componentBitSet |= (1 << INDEX);

    // TODO
  }
};

class IEntitySystem {
  public:
  int aspectIncludes;
  EcsWorld *world;
  virtual void processEntity(Entity &entity) {}

  template <class TCmp>
  TCmp &getCmp(Entity &entity);

  template <class TCmp>
  TCmp &getCmp(int entityId);

  EntityEdit& edit(Entity& entity);
};

//template <class... TComponentTypes>
class EntitySystem : public IEntitySystem {
  public:
  EntitySystem() {
    aspectIncludes = 0;
  }
  EntitySystem(int cmpIndices...) {
    va_list vl;
    va_start(vl, cmpIndices);

    int aspect;
    int i;
    do {
      i = va_arg(vl, int);
      if (i > 0)
        aspect |= (1 << i);
    } while (i > 0);

    va_end(vl);
    this->aspectIncludes = aspect;
  }
};

class EcsWorld {
  Array<Entity> entities;
  Array<Component> *components;
  Array<IEntitySystem *> systems;
  friend class EntityEdit;
  friend class IEntitySystem;

  int componentTypesCount;

  EntityEdit *reusableEntityEdit;

  public:
  EcsWorld(int componentTypesCount, IEntitySystem systems[]);

  EntityEdit &newEntity();
  EntityEdit &edit(Entity &entity);

  void process();
};

class EntityEdit {
  friend class EcsWorld;

  EcsWorld *world;
  Entity *entity;

  public:
  template <unsigned int INDEX>
  void set(IndexedComponent<INDEX> &c);

  template <class TCmp>
  TCmp &get() {
    constexpr int i = TCmp::I;
    Component &c = world->components[i][entity->id];
    return (TCmp &)c;
  }

  template <class TCmp>
  void remove() {
    constexpr int i = TCmp::I;
    //world->components[i].remove(entity->id);

    entity->componentBitSet &= ~(1 << i);
  }

  // template <
  //     template <unsigned int> class TCmp, unsigned int INDEX>
  // TCmp<INDEX> &get();
};

#endif
