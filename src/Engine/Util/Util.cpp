#include "Util.hpp"

std::string Util::stringFromVec3(glm::vec3 vec) {
  float x = vec.x;
  float y = vec.y;
  float z = vec.z;
  return "(X: " + std::to_string(x) + ", Y: " + std::to_string(y) + ", Z: " + std::to_string(z) + ")";
}

std::string Util::stringFromIVec3(glm::ivec3 vec) {
  int x = vec.x;
  int y = vec.y;
  int z = vec.z;
  return "(X: " + std::to_string(x) + ", Y: " + std::to_string(y) + ", Z: " + std::to_string(z) + ")";
}