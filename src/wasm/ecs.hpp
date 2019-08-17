#ifndef ECS__H
#define ECS__H

#include <tuple>
extern void l(int);

struct Component {};

template <unsigned int INDEX>
struct IndexedComponent : Component {};

#define DEF_COMPONENT(name) \
  struct name : IndexedComponent<__COUNTER__>



template <class T, unsigned int max_size = 1024>
class Array {
  public:
  T data[max_size];
  int size;

  Array() : size(0) {}

  void set(int index, T &el) {
    data[index] = *el;
  }

  int add(T &el) {
    data[size++] = *el;
    return size;
  }

  T &create() {
    return data[size++];
  }

  inline T &operator[](int index) {
    return data[index];
  }
};


template <typename... TComponents>
class Components {
  std::tuple<Array<TComponents>...> bags;

  public:
  typedef Components<TComponents...> type;

  template <typename T>
  T &create() {
    return getBag<T>().create();
  }

  template <typename T>
  Array<T> &getBag() {
    return std::get<Array<T>>(bags);
  }
};

struct Entity {
  int componentBitSet;

  template <unsigned int INDEX>
  void add(IndexedComponent<INDEX> &c) {
    constexpr int i = INDEX;
    // TODO
    l(i);
  }
};



// template <>
struct AAComponentMapper {

  template <unsigned int INDEX>
  void add(Entity& entity, IndexedComponent<INDEX> &c) {

    entity.componentBitSet |= (1 << INDEX);

    // TODO

  }
};
template <typename... TSystems>
class Systems {
  std::tuple<TSystems...> systems;

  public:
  const unsigned int N = sizeof...(TSystems);

  template <typename TSystem>
  TSystem& get() {
    return std::get<TSystem>(systems);
  }

};

// old
struct EntitySystem {
  int componentBitSet;
   void processEntity(Entity &entity);
};

template<class... TComponentTypes>
class AAEntitySystem {
  int aspect;

  public:
  void processEntity(Entity &entity);

  template <class T, int INDEX>
  T& getCmp(){
    IndexedComponent<INDEX> t;
    constexpr int index = INDEX;

    // TODO actually get it from somewhere?

    l(INDEX);
    return (T&)t;
  }
};




template<typename TC, typename TS, int MaxEntityCount> struct AAWorld; // primary template

// partial specialization of AAWorld
template<
  int MaxEntityCount,
  template<class...> class TComponents, class... TComponentTypes,
  template<class...> class TSystems, class... TSystemTypes
>
class AAWorld<TComponents<TComponentTypes...>, TSystems<TSystemTypes...>, MaxEntityCount> {
  Array<Entity, MaxEntityCount> entities;
  Components<TComponentTypes...> components;
  Systems<TSystemTypes...> systems;

  public:

  Entity& newEntity() {
    return entities.create();
  }

  void process() {
    for (std::size_t i = 0; i < systems.N; ++i) {
      auto &system = systems.get(i);
      system.process();
    }
  }
};



template <
    unsigned int MaxEntityCount,
    unsigned int SystemsCount,
   typename IComponents, IComponents& components
>
class EcsWorld {
  private:
  //   Array<Component, ComponentTypeCount * MaxEntityCount> components;
  // Components<TComponents...> cmps;

  public:
  Array<Entity, MaxEntityCount> entities;
  Array<EntitySystem, SystemsCount> systems;

  Entity &newEntity() {
    Entity &e = entities.create();
    //e.world = (int *)this;
    return e;
  }

  void process() {
    for (int is = 0, ns = systems.size; is < ns; ++is) {
      EntitySystem &system = systems[is];
      int systemAspect = system.componentBitSet;

      for (int ie = ie, ne = entities.size; ie < ne; ++ie) {
        Entity &entity = entities[ie];
        if ((entity.componentBitSet & systemAspect) != 0) {
          system.processEntity(entity);
        }
      }
    }
  }
};

template <unsigned int COMPONENT_TYPE_INDEX>
class ComponentMapper {
  private:
  //   EcsWorld *world;

  //   friend class EcsWorld;
};

#endif
