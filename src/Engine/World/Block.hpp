#pragma once

#include <map>

struct Material {
   uint32_t id{0};
};

namespace Materials {
  const Material AIR = Material{0};
  const Material SOLID = Material{1};
}