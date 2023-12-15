#pragma once

#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

#include "Buffer/Buffer.h"
#include "Image/Image.h"

#include "vk_mem_alloc.h"

#include <vector>
#include <iostream>

const std::vector<const char *> deviceExtensions = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  VK_KHR_MAINTENANCE1_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

struct VulkanInstance {
  VkInstance vkInstance;

  VmaAllocator vmaAllocator{};

  VkDebugUtilsMessengerEXT debugMessenger;

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device{};

  VkSurfaceKHR surface{};

  VkQueue graphicsQueue{};
  VkQueue presentQueue{};

  // SwapChain Images
  VkSwapchainKHR swapChain;
  //std::vector<VulkanImage::InternalImage> swapChainImages;
  std::vector<VkImage> swapChainImages;
  std::vector<VkImageView> swapChainImageViews;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  // Depth Buffering Images
  VulkanImage::InternalImage depthImage;

  // Texturing
  VkSampler mainSampler;

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
  std::vector<Buffers::VmaBuffer> uniformBuffers;
  std::vector<void*> uniformBuffersMapped;

  bool framebufferWasResized = false;
};
