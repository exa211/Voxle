#include "ResourceHandler.h"
#include "Logging/Logger.h"
#include "Image/Image.h"
#include "Buffer/Buffer.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

/**
 * Loads a Texture and creates VkImage, VkImageView also transitions the pipeline.
 * @param t Image Reference
 * @param path Path to texture in res folder
 */
void Resources::createTexture(VulkanImage::Image &t, const std::string &path) {
  LOG(D, "Loading image " + path);
  stbi_set_flip_vertically_on_load(true);
  stbi_uc *pixels = stbi_load(path.c_str(), &t.width, &t.height, &t.channels, STBI_rgb_alpha);
  if (!pixels) {
    LOG(W, "Could not load image " + path);
    return;
  }

  VkDeviceSize imageSize = t.width * t.height * 4;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                       stagingBufferMemory);

  // Map our stagingBuffer with our pixel data
  void *data;
  vkMapMemory(EngineData::i()->vkInstWrapper.device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(EngineData::i()->vkInstWrapper.device, stagingBufferMemory);

  // free emory
  stbi_image_free(pixels);

  // Create our image
  // VkImage is contained within Texture struct

  // VK_FORMAT_R8G8B8A8_SRGB applies a gamma correction, we don't want this I think, because it makes textures look darker
  // than they are
  VulkanImage::createImage(t.width, t.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, t.image.vkImage, t.image.vkImageMemory);

  // Transition and copy pixel buffer to VkImage struct
  // LAYOUT_UNDEFINED because we atm do not care what happens to the image
  VulkanImage::transitionImageLayout(t.image.vkImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  Buffer::copyBufferToImage(stagingBuffer, t.image.vkImage, t.width, t.height);
  // Tells vulkan that the image will be read by shaders
  VulkanImage::transitionImageLayout(t.image.vkImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(EngineData::i()->vkInstWrapper.device, stagingBuffer, nullptr);
  vkFreeMemory(EngineData::i()->vkInstWrapper.device, stagingBufferMemory, nullptr);

  // ----- IMAGE VIEW CREATION ----

  LOG(D, "Creating ImageView for texture: " + path);
  VulkanImage::createImageView(t.image.vkImage, t.image.vkImageView, VK_FORMAT_R8G8B8A8_UNORM);

  LOG(D, "Created Texture " + path);
}
