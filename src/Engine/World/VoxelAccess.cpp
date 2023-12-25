#include "VoxelAccess.hpp"
#include "Util/Util.hpp"

Material VoxelBoundaryAccess::getBlockAcross(Chunk *c, Direction dir, const glm::ivec3 &pos) {
  if (c == nullptr) return Materials::SOLID;

  glm::ivec3 flippedPos{};

  switch (dir) {
    case Direction::NORTH: { // -Z
      flippedPos = {pos.x, pos.y, 0};
    }
      break;
    case Direction::EAST: { // +X
      flippedPos = {0, pos.y, pos.z};
    }
      break;
    case Direction::SOUTH: { // +Z
      flippedPos = {pos.x, pos.y, CHUNK_SIZE};
    }
      break;
    case Direction::WEST: { // -X
      flippedPos = {CHUNK_SIZE, pos.y, pos.z};
    }
      break;
    case Direction::UP: { // +Y
      flippedPos = {pos.x, 0, pos.z};
    }
      break;
    case Direction::DOWN: { // -Y
      flippedPos = {pos.x, CHUNK_SIZE, pos.z};
    }
      break;
  }

  Material mAcross = c->getBlock(flippedPos.x, flippedPos.y, flippedPos.z);
  //LOG(D, "Material at " + Util::stringFromIVec3(flippedPos) + " is: " + std::to_string(mAcross.id));
  return mAcross;
}