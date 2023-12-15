#pragma once

#include <map>

struct Material {
  std::string Namespace{"default"};

  void setType(const Material& mat) {
    Namespace = mat.Namespace;
  }
};

namespace Materials {
  const Material AIR = Material{"voxle:air"};
}