#include "ResourceHandler.h"
#include "Logger.h"
#include "Texture/Texture.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

void Resource::loadTexture(Texture &t, const std::string &path) {
  LOG::ignored("Loading image " + path);
  stbi_uc *pixels = stbi_load(path.c_str(), &t.width, &t.height, &t.channels, STBI_rgb_alpha);
  if (!pixels) LOG::fatal("Could not load image " + path);

  VkDeviceSize imageSize = t.width * t.height * 4;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  Buffer::createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                       stagingBufferMemory);

  void *data;
  vkMapMemory(E_Data::i()->vkInstWrapper.device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(E_Data::i()->vkInstWrapper.device, stagingBufferMemory);

  stbi_image_free(pixels);

  TextureLoading::createTexture(t.width, t.height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, t.image, t.imageMemory);

  Buffer::copyBufferToImage(stagingBuffer, t.image, t.width, t.height);

  LOG::ignored("Created Texture " + path);
}
