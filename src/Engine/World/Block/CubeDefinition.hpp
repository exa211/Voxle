#pragma once

#include <vector>

inline const unsigned int faceIndices[] = {
  0, 1, 2,
  2, 3, 0
};

// FRONT FACE ------
inline const std::vector<signed char> frontFace = {
  0, 0, 1,// 0 lower left
  1, 0, 1, // 1 lower right
  1, 1, 1, // 2 upper right
  0, 1, 1, // 3 upper left
};
// BACK FACE ------
inline const std::vector<signed char> backFace = {
  1, 0, 0,
  0, 0, 0,
  0, 1, 0,
  1, 1, 0,
};
// TOP FACE ------
inline const std::vector<signed char> topFace = {
  0, 1, 1, // Top front left // 0
  1, 1, 1, // 1
  1, 1, 0, // 2
  0, 1, 0, // 3
};
// BOTTOM FACE ------
inline const std::vector<signed char> botFace = {
  0, 0, 0,
  1, 0, 0,
  1, 0, 1,
  0, 0, 1,
};
// LEFT FACE ------
inline const std::vector<signed char> leftFace = {
  0, 0, 0,
  0, 0, 1,
  0, 1, 1,
  0, 1, 0,
};
// RIGHT FACE ------
inline const std::vector<signed char> rightFace = {
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  1, 1, 1,
};