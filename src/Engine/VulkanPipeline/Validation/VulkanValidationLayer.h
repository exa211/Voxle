#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>
#include <cstring>

const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidation = false;
#else
const bool enableValidation = true;
#endif

namespace VulkanValidation {

  bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for(const char* layerName : validationLayers) {
      bool layerFound = false;
      for(const auto& layerProps : availableLayers) {
        if(strcmp(layerName, layerProps.layerName) == 0) {
          layerFound = true;
          break;
        }
      }
      if(!layerFound) return false;
    }
    return true;
  }

  std::vector<const char*> getRequireExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if(enableValidation) {
      extensions.push_back("VK_KHR_portability_enumeration");
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
  }

}