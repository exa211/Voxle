#pragma once

#include "GLFW/glfw3.h"
#include <string>

struct EngineWindow {
  std::string title;
  GLFWwindow* window;
};