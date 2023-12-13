#pragma once

#include "vulkan/vulkan.h"

namespace VulkanImage {

  struct Image {
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView view;
    int width;
    int height;
    int channels;
  };

  void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

  VkImageView createImageView(VkImage image, VkFormat format);
}

