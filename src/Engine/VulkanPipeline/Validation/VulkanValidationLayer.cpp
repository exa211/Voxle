#include "VulkanValidationLayer.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstring>

bool VulkanValidation::checkValidationLayerSupport() {
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

std::vector<const char*> VulkanValidation::getRequireExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
  if(enableValidation) {
    extensions.push_back("VK_KHR_portability_enumeration");
    //extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  return extensions;
}