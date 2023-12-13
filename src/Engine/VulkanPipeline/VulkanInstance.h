#pragma once

#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>

const std::vector<const char *> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanInstance {
public:
  VkInstance vkInstance;

  VkDebugUtilsMessengerEXT debugMessenger;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device{};

  VkSurfaceKHR surface{};

  VkQueue graphicsQueue{};
  VkQueue presentQueue{};

  // Render Images
  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  // Depth Buffering Images
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

  VkExtent2D extent{};
  VkFormat format;

  VkPipeline pipeline;
  VkPipelineLayout pipelineLayout;
  VkRenderPass renderPass;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  std::vector<VkSemaphore> imageAvailableSemas;
  std::vector<VkSemaphore> renderFinishedSemas;
  std::vector<VkFence> inFlightFences;

  // Objects for immediate command submissions, used in UI creation stuff
  VkFence immediateUploadFence;
  VkCommandPool immediateUploadPool;
  VkCommandBuffer immediateCommandBuffer;

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> descriptorSets;

  // Uniform Buffer Objects
  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBufferMemory;
  std::vector<void*> uniformBuffersMapped;

  bool framebufferWasResized = false;
};
