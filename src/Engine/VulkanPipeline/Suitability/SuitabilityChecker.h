#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace SuitabilityChecker {
  bool checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char*>& extensions);
  bool isDeviceSuitable(VkPhysicalDevice device);
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props);

  // Depth Buffer Format checking and getting
  VkFormat findSupportedDepthFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
  VkFormat getSupportedDepthFormat();
  bool hasDepthStencilComponent(VkFormat format);
}
