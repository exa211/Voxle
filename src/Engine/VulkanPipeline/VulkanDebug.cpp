#include "VulkanDebug.h"
#include "Logging/Logger.h"

#include <iostream>

#include <vulkan/vulkan.h>

VkResult VulkanDebug::createDebugUtilsMessengerEXT(VkInstance instance,
                                               const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator,
                                               VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto function = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if(function != nullptr) {
    return function(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanDebug::destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                const VkAllocationCallbacks *pAllocator) {
  auto function = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if(function != nullptr) {
    function(instance, debugMessenger, pAllocator);
  }
}