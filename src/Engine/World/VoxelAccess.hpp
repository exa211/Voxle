#pragma once

#include "Chunk.hpp"

enum class Direction {
  NORTH,
  EAST,
  SOUTH,
  WEST,
  UP,
  DOWN
};

namespace VoxelBoundaryAccess {
  Material getBlockAcross(Chunk* c, Direction dir, const glm::ivec3& pos);
}