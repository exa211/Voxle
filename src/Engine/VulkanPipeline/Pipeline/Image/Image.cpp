#include "Image.h"

#include "Engine.h"
#include "Logging/Logger.h"
#include "VulkanPipeline/Suitability/SuitabilityChecker.h"
#include "VulkanPipeline/Pipeline/Commandbuffer.h"
#include "VulkanPipeline/Pipeline/GraphicsPipeline.h"

void VulkanImage::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
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

  if (vkCreateImage(EngineData::i()->vkInstWrapper.device, &imageInfo, nullptr, &image) != VK_SUCCESS)
  LOG(F, "Failed to create VkImage");

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(EngineData::i()->vkInstWrapper.device, image, &memRequirements);

  VkMemoryAllocateInfo allocateInfo{};
  allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocateInfo.allocationSize = memRequirements.size; // <- Size of image texture
  allocateInfo.memoryTypeIndex = SuitabilityChecker::findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(EngineData::i()->vkInstWrapper.device, &allocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
    LOG(F, "Could not allocate Image Memory");

  vkBindImageMemory(EngineData::i()->vkInstWrapper.device, image, imageMemory, 0);
}

void VulkanImage::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
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

  // Depth Format | Depth Stencil
  if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if(SuitabilityChecker::hasDepthStencilComponent(format)) {
      barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  } else {
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  }

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
  } else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    graphicsPipeline.sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    // Earliest stage of rendering because we want to depth test only against bare vertices and nothing else
    graphicsPipeline.dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  } else {
    LOG(W, "Currently transitioning into unsupported layout");
  }

  // Signal pipeline to wait
  vkCmdPipelineBarrier(singleUseCmdBuffer,
                       graphicsPipeline.sourceStage,
                       graphicsPipeline.dstStage,
                       0, 0, nullptr, 0, nullptr, 1, &barrier);

  Commandbuffer::endRecordSingleTime(singleUseCmdBuffer);
}

void VulkanImage::createImageView(VkImage image, VkImageView& imageView, VkFormat format, VkImageAspectFlags aspectFlag) {
  LOG(W, image == VK_NULL_HANDLE, "Image is NULL (createImageView Image.cpp)");

  VkImageViewCreateInfo viewCreateInfo{};
  viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewCreateInfo.image = image;
  viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewCreateInfo.format = format;
  viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  viewCreateInfo.subresourceRange.aspectMask = aspectFlag;
  viewCreateInfo.subresourceRange.baseMipLevel = 0;
  viewCreateInfo.subresourceRange.levelCount = 1;
  viewCreateInfo.subresourceRange.baseArrayLayer = 0;
  viewCreateInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(EngineData::i()->vkInstWrapper.device, &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
    LOG(F, "Failed to create Image Views");
}

/**
 *  Creates a TextureSampler with given settings,
 *  repeatMode is for UV(W) tiling
 **/
void VulkanImage::Sampler::createTextureSampler(VkSampler& sampler, VkFilter filter,
                                                VkSamplerAddressMode repeatMode) {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = filter;
  samplerInfo.minFilter = filter;
  // Consider making new params for every single coord.
  samplerInfo.addressModeU = repeatMode;
  samplerInfo.addressModeV = repeatMode;
  samplerInfo.addressModeW = repeatMode;

  VkPhysicalDeviceProperties devicePropsAnis{};
  vkGetPhysicalDeviceProperties(EngineData::i()->vkInstWrapper.physicalDevice, &devicePropsAnis);

  samplerInfo.anisotropyEnable = VK_TRUE; // <- I personally think this should always be ton
  // I think we should later implement settings for this because this can get performance heavy on older cards!
  samplerInfo.maxAnisotropy = devicePropsAnis.limits.maxSamplerAnisotropy;

  // UV Stuffs
  // Border Color BLACK for when the uv is going beyond 1.0 coordinate
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE; // <- Don't really know what this looks like...

  // Can get interesting in shadow mapping later
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

  // Mip-mapping I think this explains itself.
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if(vkCreateSampler(EngineData::i()->vkInstWrapper.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    LOG(F, "Sampler could not be created (Image.cpp)");
}
