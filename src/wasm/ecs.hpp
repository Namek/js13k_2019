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

// old
struct EntitySystem {
  int componentBitSet;
   void processEntity(Entity &entity);
};

class IAAEntitySystem {
  public:
  int aspect;
  void processEntity(Entity &entity);
};

template<class... TComponentTypes>
class AAEntitySystem : public IAAEntitySystem {


public:
  template <class T, int INDEX>
  T& getCmp(){
    IndexedComponent<INDEX> t;
    constexpr int index = INDEX;

    // TODO actually get it from somewhere?

    l(INDEX);
    return (T&)t;
  }

   void test();

};


////-------------

/*
#include <tuple>
#include <iostream>

template<typename T>
void doSomething(T t) { std::cout << t << '\n';}

template<int... N>
struct Switch;

template<int N, int... Ns>
struct Switch<N, Ns...>
{
  template<typename... T>
    void operator()(int n, std::tuple<T...>& t)
    {
      if (n == N)
        doSomething(std::get<N>(t));
      else
        Switch<Ns...>()(n, t);
    }
};

// default
template<>
struct Switch<>
{
  template<typename... T>
    void operator()(int n, std::tuple<T...>& t) { }
};

template<size_t... N>
Switch<N...>
make_switch(std::index_sequence<N...>)
{
  return {};
}
*/
///----------------

// source:
//https://www.justsoftwaresolutions.co.uk/cplusplus/getting-tuple-elements-with-runtime-index.html

// #include <tuple>
// #include <utility>
// #include <type_traits>
// #include <stdexcept>

// template<
//   typename Tuple,
//   typename Indices=std::make_index_sequence<std::tuple_size<Tuple>::value>>
// struct runtime_get_func_table;

// template<typename Tuple,size_t ... Indices>
// struct runtime_get_func_table<Tuple,std::index_sequence<Indices...>>{
//     using return_type=typename std::tuple_element<0,Tuple>::type&;
//     using get_func_ptr=return_type (*)(Tuple&) noexcept;
//     static constexpr get_func_ptr table[std::tuple_size<Tuple>::value]={
//         &std::get<Indices>...
//     };
// };

// template<typename Tuple,size_t ... Indices>
// constexpr typename
// runtime_get_func_table<Tuple,std::index_sequence<Indices...>>::get_func_ptr
// runtime_get_func_table<Tuple,std::index_sequence<Indices...>>::table[std::tuple_size<Tuple>::value];

// template<typename Tuple>
// constexpr
// typename std::tuple_element<0,typename std::remove_reference<Tuple>::type>::type&
// runtime_get(Tuple&& t,size_t index){
//     using tuple_type=typename std::remove_reference<Tuple>::type;
//     if(index>=std::tuple_size<tuple_type>::value)
//         throw std::runtime_error("Out of range");
//     return runtime_get_func_table<tuple_type>::table[index](t);
// }



//-----------------
//https://stackoverflow.com/a/47563100/450926

// template<std::size_t N>
// struct num { static const constexpr auto value = N; };

// template <class F, std::size_t... Is>
// void for_(F func, std::index_sequence<Is...>)
// {
//   using expander = int[];
//   (void)expander{0, ((void)func(num<Is>{}), 0)...};
// }

// template <std::size_t N, typename F>
// void for_(F func)
// {
//   for_(func, std::make_index_sequence<N>());
// }

//-------------------

//https://stackoverflow.com/a/26902803/450926

template<class F, class...Ts, std::size_t...Is>
void for_each_in_tuple( std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
void for_each_in_tuple(std::tuple<Ts...> & tuple, F func){
    for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

//-------------------



template <typename... TSystems>
class Systems {
  std::tuple<TSystems...> systems;

  public:
  IAAEntitySystem* indexed;
  const unsigned int N = sizeof...(TSystems);

  Systems() {
    indexed = new IAAEntitySystem[N];
    unsigned int i = 0;
    for_each_in_tuple(systems, [this, &i](IAAEntitySystem& s) {
      this->indexed[i++] = s;
    });
  }


  template <typename TSystem>
  // template constexpr std::is_convertible<TSystem, EntitySystem>
  decltype(auto) get() {
    return (IAAEntitySystem&)std::get<TSystem>(systems);
  }

  inline IAAEntitySystem& get(unsigned int index) {
    return (IAAEntitySystem&) indexed[index];
  }
};


template<typename TC, typename TS, int MaxEntityCount> struct World; // primary template

// partial specialization of AAWorld
template<
  int MaxEntityCount,
  template<class...> class TComponents, class... TComponentTypes,
  template<class...> class TSystems, class... TSystemTypes
>
class World<TComponents<TComponentTypes...>, TSystems<TSystemTypes...>, MaxEntityCount> {
  Array<Entity, MaxEntityCount> entities;
  Components<TComponentTypes...> components;
  Systems<TSystemTypes...> systems;

  public:

  Entity& newEntity() {
    return entities.create();
  }

  void process() {
    for (int i = 0; i < systems.N; ++i) {
      auto system = systems.get(i);
      int systemAspect = system.aspect;

      for (int ie = ie, ne = entities.size; ie < ne; ++ie) {
        Entity &entity = entities[ie];
        if ((entity.componentBitSet & systemAspect) != 0) {
          system.processEntity(entity);
        }
      }
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
