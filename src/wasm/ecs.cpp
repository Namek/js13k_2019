#include "ecs.hpp"

// #include "engine.hpp"

// template <class... ComponentTypes> struct Components {
//     public:
//     void init(Array<ComponentTypes>... arrays ) {
//         arrays
//     }
// };



DEF_COMPONENT(Vehicle) {
  int laneIndex;
};

DEF_COMPONENT(Level) {
  int laneCount;
};

__attribute__((used)) void omg() {
  Components<Vehicle, Level> omg;
  Level &l = omg.create<Level>();
}