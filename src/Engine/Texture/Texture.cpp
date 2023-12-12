#include "Texture.h"

void TextureLoading::createTexture(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                                   VkImageUsageFlags usage,
                                   VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(E_Data::i()->vkInstWrapper.device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    LOG::fatal("Failed to create VkImage");

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(E_Data::i()->vkInstWrapper.device, image, &memRequirements);

  VkMemoryAllocateInfo allocateInfo{};
  allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocateInfo.allocationSize = memRequirements.size;
  allocateInfo.memoryTypeIndex = SuitabilityChecker::findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(E_Data::i()->vkInstWrapper.device, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
    LOG::fatal("Could not allocate Image Memory");

  vkBindImageMemory(E_Data::i()->vkInstWrapper.device, image, imageMemory, 0);

  transitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
}

void TextureLoading::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
                                           VkImageLayout newLayout) {
  VkCommandBuffer singleUseCmdBuffer = Commandbuffer::recordSingleTime();

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;

  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    graphicsPipeline.sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    graphicsPipeline.dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    graphicsPipeline.sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    graphicsPipeline.dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    LOG::fatal("Unsupported layout transition. In (Texture.cpp)");
  }

  vkCmdPipelineBarrier(singleUseCmdBuffer, graphicsPipeline.sourceStage, graphicsPipeline.dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  Commandbuffer::endRecordSingleTime(singleUseCmdBuffer);
}
