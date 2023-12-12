#pragma once

#ifndef VOXELATE_COLOR_H
#define VOXELATE_COLOR_H

#include <glm/glm.hpp>

class Color {
public:
  float r, g, b;

  explicit Color(glm::vec3 color) {
    r = color.x / 255.0f;
    g = color.y / 255.0f;;
    b = color.z / 255.0f;;
  }

  explicit Color(int r, int g, int b) {
    this->r = (float) r / 255.0f;
    this->g = (float) g / 255.0f;;
    this->b = (float) b / 255.0f;;
  }

};

#endif //VOXELATE_COLOR_H
