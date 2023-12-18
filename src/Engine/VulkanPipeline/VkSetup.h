#pragma once
#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

#include <Logging/Logger.h>

#include <vector>
#include <iostream>

#include "Image/Image.h"

namespace VkSetup {
  void createVulkanInstance();
  void createDebugMessenger();

  void createSurface();

  void selectPhysicalDevice();
  void createLogicalDevice();

  void createVmaAllocator();

  void createSwapchain(bool isResize = false);
  void recreateSwapchain(VkDevice& device);
  void cleanupOldSwapchain(VkDevice& device);

  void createImageViews();
  void createSampler();

  VkDescriptorSetLayout createDescriptorSetLayout();
  void createDescriptorPool();
  void createDescriptorSets();

  void populateDescriptors(VulkanImage::InternalImage& image);
}
