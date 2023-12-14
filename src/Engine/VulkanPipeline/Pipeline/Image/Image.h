#pragma once

#include "vulkan/vulkan.h"

namespace VulkanImage {

  struct InternalImage {
    VkImage vkImage;
    VkDeviceMemory vkImageMemory;
    VkImageView vkImageView;
  };

  struct Image {
    InternalImage image;
    int width;
    int height;
    int channels;
  };

  void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);

  void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

  void createImageView(VkImage image, VkImageView& imageView, VkFormat format, VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT);

  namespace Sampler {

    void createTextureSampler(VkSampler &sampler, VkFilter filter, VkSamplerAddressMode repeatMode);

  }

}

