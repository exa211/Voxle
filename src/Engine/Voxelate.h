#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <VulkanPipeline/VulkanInstance.h>
#include <Engine.h>

#include <VulkanPipeline/VkSetup.h>
#include "VulkanPipeline/Pipeline/GraphicsPipeline.h"
#include "Scene/Scene.h"

#include <Logging/Logger.h>

#include <string>
#include <iostream>

#include "Camera/Camera.h"

const uint8_t W_SCALING = 2;

const uint32_t W_WIDTH = 640 * W_SCALING;
const uint32_t W_HEIGHT = 370 * W_SCALING;

class Voxelate {
public:
  Camera cam{W_WIDTH, W_HEIGHT};
  void run();

private:
  void initEngine();

  void initWindow();

  void initVulkan();

  void initScene();

  void initGui();

  void update(float deltaTime);
  void loop();

  void clean();
};
