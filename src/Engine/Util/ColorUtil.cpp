#include "ColorUtil.hpp"


glm::vec3 ColorUtil::convertRGBtoFloat(glm::vec3 inRgb) {
  return {inRgb.x / 255.0f, inRgb.y / 255.0f, inRgb.z / 255.0f};
}