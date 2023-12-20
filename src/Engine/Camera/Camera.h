#pragma once

#include <glm/glm.hpp>
#include "GLFW/glfw3.h"

class Camera {
public:
  Camera(int width, int height) : width(width), height(height) {}

  glm::vec3 position{0.0f};
  glm::vec3 direction{0.0, 0.0, -1.0};
  glm::vec3 up{0.0, 1.0, 0.0};

  // TODO Update this on window resize
  int width;
  int height;

  struct CameraMatrix {
    glm::mat4 view;
    glm::mat4 proj;
  } cameraMatrix{};

  bool mouseCaptured = false;
  float mouseSensitivity = 200.0f;
  float flySpeed = 15.0f;

  float fov = 90;
  float nearPlane = 0.1f;
  float farPlane = 1000.0f;

  void update(GLFWwindow* window, float& deltaTime);
};
