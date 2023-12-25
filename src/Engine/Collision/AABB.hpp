#pragma once

#include <utility>
#include "Renderer/Primitives/MeshPrimitives.h"

struct Point {
  float x;
  float y;
  float z;
};

class AABB {
public:
  AABB(Point center, Point halfWidth) : center(center), halfWidth(halfWidth) {}

  bool isColliding(AABB& collider) const;

  std::pair<std::vector<Vertex>, std::vector<uint32_t>> getMeshDefinition();

private:
  AABB() : center(), halfWidth() {}
  Point center{};
  Point halfWidth{};
};