#include "Camera.h"


#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include <imgui.h>
#include <Logging/Logger.h>

void Camera::update(GLFWwindow* window, float& deltaTime) {

  //TODO: Use custom InputEventDispatchers so we can defer this
  ImGuiIO &io = ImGui::GetIO();
  (void) io;
  if(io.WantCaptureMouse) return;

  if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    position += (flySpeed * direction) * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    position -= glm::normalize(glm::cross(direction, up)) * flySpeed * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    position -= (flySpeed * direction) * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    position += glm::normalize(glm::cross(direction, up)) * flySpeed * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    position += glm::normalize(up) * flySpeed * deltaTime;
  if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    position -= glm::normalize(up) * flySpeed * deltaTime;

  if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    if(!mouseCaptured) {
      glfwSetCursorPos(window, (float)width/2, (float)height/2);
      mouseCaptured = true;
    }
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    float rotX = mouseSensitivity * (float)(mouseY - (float)height / 2) / (float)height;
    float rotY = mouseSensitivity * (float)(mouseX - (float)width / 2) / (float)width;

    glm::vec3 newOrientation = glm::rotate(direction, glm::radians(-rotX), glm::normalize(glm::cross(direction, up)));
    // Clamp values

    long ori = abs(glm::angle(newOrientation, up) - glm::radians(90.0f));

    if(ori <= glm::radians(90.0f)) {
      direction = newOrientation;
    }
      direction = glm::rotate(direction, glm::radians(-rotY), up);

    glfwSetCursorPos(window, (float)width / 2, (float)height / 2);
  } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    mouseCaptured = false;
  }

  cameraMatrix.view = glm::lookAt(position, position + direction, up);
  cameraMatrix.proj = glm::perspective(glm::radians(fov), (float)width/(float)height, nearPlane, farPlane);
  cameraMatrix.proj[1][1] *= -1; // Invert the coordinate system
}
