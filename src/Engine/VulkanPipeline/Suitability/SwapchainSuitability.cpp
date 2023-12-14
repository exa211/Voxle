#include <limits>
#include <functional>
#include <algorithm>
#include "SwapchainSuitability.h"
#include "Logging/Logger.h"

SwapChainSupportDetails SwapchainSuitability::querySwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR &surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.caps);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkExtent2D SwapchainSuitability::chooseExtent(VkSurfaceCapabilitiesKHR &caps, int w_frameBuffer, int h_frameBuffer) {
  if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) return caps.currentExtent;

  VkExtent2D actualExtent = {
    static_cast<uint32_t>(w_frameBuffer),
    static_cast<uint32_t>(h_frameBuffer)
  };

  actualExtent.width = std::clamp(actualExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
  actualExtent.height = std::clamp(actualExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
  return actualExtent;
}

VkSurfaceFormatKHR SwapchainSuitability::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto& format : availableFormats) {
    if (format.format == VK_FORMAT_R32G32B32A32_SFLOAT && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      LOG(I, "Using Surface Format: " + std::to_string(format.format));
      return format;
    }
  }
  LOG(I, "Using Surface Format: " + std::to_string(availableFormats[0].format));
  return availableFormats[0];
}

VkPresentModeKHR SwapchainSuitability::choosePresentMode(const std::vector<VkPresentModeKHR> &availableModes) {
  for (const auto& mode : availableModes) {
    LOG(I, "Available PRESENT MODE: " + std::to_string(mode));
    if (mode == VK_PRESENT_MODE_FIFO_KHR) {
      return mode;
    }
  }
  LOG(I, "Using PRESENT MODE: " + std::to_string(availableModes[0]));
  return VK_PRESENT_MODE_FIFO_KHR;
}
