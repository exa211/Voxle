#pragma once
#ifndef VOXELATE_GENERICPUSHCONSTANTS_H
#define VOXELATE_GENERICPUSHCONSTANTS_H

#include "glm/glm.hpp"

struct MeshPushConstant {
  glm::vec4 data;
  glm::mat4 transformMatrix{1};
};

#endif //VOXELATE_GENERICPUSHCONSTANTS_H
