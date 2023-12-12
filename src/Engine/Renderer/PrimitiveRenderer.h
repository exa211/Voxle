#pragma once

#include <imgui_impl_vulkan.h>

#include <Engine.h>
#include <Logging/Logger.h>
#include "../VulkanPipeline/VkSetup.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include "Camera/Camera.h"

namespace PrimitiveRenderer {
  void render(Camera& cam);
  void updateUniformBuffers(uint32_t currentFrame, Camera& cam);
}