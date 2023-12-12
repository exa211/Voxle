#include <vector>
#include "QueueHelper.h"

QueueFamilyIndices QueueHelper::findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
  QueueFamilyIndices indices{};

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  int i = 0;

  for (const auto &queueFamily: queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

      if (presentSupport) {
        indices.presentFamily = i;
      }
      indices.graphicsFamily = i;

      if (indices.hasBoth()) break;
    }
    i++;
  }

  return indices;
}