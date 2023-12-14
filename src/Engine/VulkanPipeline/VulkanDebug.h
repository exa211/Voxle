#pragma once

#include <vulkan/vulkan.h>
#include "Logging/Logger.h"

/**
*  This handles debugCallbackMessages from Vulkan
**/

namespace VulkanDebug {

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *pData,
    void *pUserData
  ) {

    if (type == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
      LOG(W, "Vulkan Un-optimized: " + std::string(pData->pMessage));
    }

    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      LOG(W, "Validation Layer: " + std::string(pData->pMessage));
    }

    return VK_FALSE;
  }

  /**
   * This looks up the function address and creates a proxy function for it.
   **/
  VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator,
                                        VkDebugUtilsMessengerEXT* pDebugMessenger);

  /**
   * This looks up the function address and creates a proxy function for it.
   **/
  void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

}