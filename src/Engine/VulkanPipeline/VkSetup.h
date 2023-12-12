#pragma once

#include "GLFW/glfw3.h"
#include <glad/vulkan.h>

#include <Engine.h>
#include <Logging/Logger.h>
#include "Pipeline/GraphicsPipeline.h"

#include <vector>
#include <iostream>

namespace VkSetup {
  void createVulkanInstance();

  void createSurface();

  void selectPhysicalDevice();

  void createLogicalDevice();

  void createSwapchain(bool isResize = false);

  void recreateSwapchain(VkDevice& device);

  void cleanupOldSwapchain(VkDevice& device);

  void createImageViews();

  VkDescriptorSetLayout createDescriptorSetLayout();
  void createDescriptorPool();
  void createDescriptorSets();

  void populateDescriptors();

}
