#pragma once
#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

#include <Engine.h>
#include <Logging/Logger.h>
#include "Pipeline/GraphicsPipeline.h"

#include <vector>
#include <iostream>

namespace VkSetup {
  void createVulkanInstance();
  void createDebugMessenger();

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
