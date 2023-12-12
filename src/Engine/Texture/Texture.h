#pragma once

#include "Engine.h"
#include "Logging/Logger.h"
#include "VulkanPipeline/Suitability/SuitabilityChecker.h"
#include "VulkanPipeline/Pipeline/Commandbuffer.h"
#include "VulkanPipeline/Pipeline/Buffer/Buffer.h"
#include "VulkanPipeline/Pipeline/GraphicsPipeline.h"

#include "vulkan/vulkan.h"

struct Texture {
  VkImage image;
  VkDeviceMemory imageMemory;
  int width;
  int height;
  int channels;
};

namespace TextureLoading {
  void createTexture(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
}

