#include <set>
#include <string>
#include <Engine.h>
#include <Logging/Logger.h>
#include "SuitabilityChecker.h"

/*
 *  Checks if the given device supports the given extensions
 **/
bool
SuitabilityChecker::checkDeviceExtensionSupport(VkPhysicalDevice device, const std::vector<const char *> &extensions) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

  for (const auto &extension: availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

bool SuitabilityChecker::isDeviceSuitable(VkPhysicalDevice device) {
  return true;
}

/*
 *  VkPhysicalDeviceMemoryProperties has 2 arrays "memoryTypes" and "memoryHeaps"
 *  Memory heaps are memory resources like VRAM and Swap Space in RAM
 **/
uint32_t SuitabilityChecker::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) {
  VkPhysicalDeviceMemoryProperties memoryProperties;
  vkGetPhysicalDeviceMemoryProperties(E_Data::i()->vkInstWrapper.physicalDevice, &memoryProperties);

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if (typeFilter & (1 << i) && (memoryProperties.memoryTypes[i].propertyFlags & props) == props) {
      return i;
    }
  }

  LOG::fatal("Cannot find suitable memory type on selected GPU");
  return 0;
}

VkFormat SuitabilityChecker::findSupportedDepthFormat(const std::vector<VkFormat> &candidates,
                                                      VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (VkFormat format: candidates) {
    VkFormatProperties prop;
    vkGetPhysicalDeviceFormatProperties(E_Data::i()->vkInstWrapper.physicalDevice, format, &prop);

    if (tiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (prop.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  LOG::fatal("Could not find a supported depth buffer format");
}

VkFormat SuitabilityChecker::getSupportedDepthFormat() {
  return SuitabilityChecker::findSupportedDepthFormat(
    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool SuitabilityChecker::hasDepthStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
