#include "AABB.hpp"

#include <cmath>

constexpr inline float Abs(float val) {
  return std::fabs(val);
}

/**
 * @brief Checks if the given collider collides with this AABB
 * This is SIMD optimized
 * */
bool AABB::isColliding(AABB &collider) const {
  bool x = Abs(this->center.x - collider.center.x) <= (this->halfWidth.x - collider.halfWidth.x);
  bool y = Abs(this->center.y - collider.center.y) <= (this->halfWidth.y - collider.halfWidth.y);
  bool z = Abs(this->center.z - collider.center.z) <= (this->halfWidth.z - collider.halfWidth.z);
  return x && y && z;
}

std::pair<std::vector<Vertex>, std::vector<uint32_t>> AABB::getMeshDefinition() {

  float minX = center.x - halfWidth.x;
  float minY = center.y - halfWidth.y;
  float minZ = center.z - halfWidth.z;

  float maxX = center.x + halfWidth.x;
  float maxY = center.y + halfWidth.y;
  float maxZ = center.z + halfWidth.z;

  std::vector<Vertex> vertices = {
    Vertex{{minX, minY, minZ}, {0, 0, 0}, {0, 0}},
    Vertex{{maxX, minY, minZ}, {0, 0, 0}, {0, 0}},
    Vertex{{maxX, maxY, minZ}, {0, 0, 0}, {0, 0}},
    Vertex{{minX, maxY, minZ}, {0, 0, 0}, {0, 0}}
  };

  std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
  };

  return {vertices, indices};
}
