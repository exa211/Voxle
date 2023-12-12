#pragma once

#include <glad/vulkan.h>
#include <optional>

struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool hasBoth() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

namespace QueueHelper {
  QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &device, const VkSurfaceKHR &surface);
}