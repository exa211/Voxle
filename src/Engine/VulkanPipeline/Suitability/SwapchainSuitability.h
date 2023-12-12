#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include <Engine.h>

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR caps;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

namespace SwapchainSuitability {
  SwapChainSupportDetails querySwapChainDetails(VkPhysicalDevice device, VkSurfaceKHR& surface);
  VkExtent2D chooseExtent(VkSurfaceCapabilitiesKHR &caps, int w_frameBuffer, int h_frameBuffer);
  VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availableModes);
}
