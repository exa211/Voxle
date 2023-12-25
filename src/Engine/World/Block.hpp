#pragma once

#include <map>
#include <Collision/AABB.hpp>

struct Material {
   uint32_t id{0};
};

namespace Materials {
  const Material AIR = Material{0};
  const Material SOLID = Material{1};
}

class Block {
public:
  Material type;
  AABB aabb;
};