#include "Buffer.h"

VertexBuffer Buffer::createVertexBuffer(const std::vector<Vertex> &vertices) {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  // Create the Staging VkBuffer
  // This is used to transfer the given vertex data into high performance memory from the graphics card
  VertexBuffer stagingBuffer{};
  stagingBuffer.size = bufferSize;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer,
               stagingBuffer.bufferMemory);

  // This is mapping the buffer memory to CPU accessible memory
  void *data;
  vkMapMemory(device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), bufferSize);
  vkUnmapMemory(device, stagingBuffer.bufferMemory);

  // Construct our wrapper VertexBuffer structure
  VertexBuffer vb{};
  vb.size = bufferSize;

  // Create the local device buffer on the gpu
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb.buffer, vb.bufferMemory);
  // Copy the vertex data to the dst local vertex buffer ()
  copyBuffer(stagingBuffer.buffer, vb.buffer, bufferSize);

  // Free the resources used by the staging buffer
  vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
  vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

  return vb;
}

VertexBuffer Buffer::createBlockVertexBuffer(const std::vector<BlockVertex> &blockVertices) {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;

  VkDeviceSize bufferSize = sizeof(blockVertices[0]) * blockVertices.size();

  // Create the Staging VkBuffer
  // This is used to transfer the given vertex data into high performance memory from the graphics card
  VertexBuffer stagingBuffer{};
  stagingBuffer.size = bufferSize;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer,
               stagingBuffer.bufferMemory);

  // This is mapping the buffer memory to CPU accessible memory
  void *data;
  vkMapMemory(device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, blockVertices.data(), bufferSize);
  vkUnmapMemory(device, stagingBuffer.bufferMemory);

  // Construct our wrapper VertexBuffer structure
  VertexBuffer vb{};
  vb.size = bufferSize;

  // Create the local device buffer on the gpu
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb.buffer, vb.bufferMemory);
  // Copy the vertex data to the dst local vertex buffer ()
  copyBuffer(stagingBuffer.buffer, vb.buffer, bufferSize);

  // Free the resources used by the staging buffer
  vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
  vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

  return vb;
}

IndexBuffer Buffer::createIndexBuffer(const std::vector<uint32_t> &indices) {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VertexBuffer stagingBuffer{};
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer,
               stagingBuffer.bufferMemory);

  // This is mapping the buffer memory to CPU accessible memory
  void *data;
  vkMapMemory(device, stagingBuffer.bufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vkUnmapMemory(device, stagingBuffer.bufferMemory);

  // Local index buffer
  IndexBuffer localBuffer{};
  localBuffer.indicesSize = indices.size();

  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, localBuffer.indexBuffer, localBuffer.indexBufferMemory);
  copyBuffer(stagingBuffer.buffer, localBuffer.indexBuffer, bufferSize);

  vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
  vkFreeMemory(device, stagingBuffer.bufferMemory, nullptr);

  return localBuffer;
}

void Buffer::createUniformBuffers() {
  VkDeviceSize size = sizeof(UniformBufferObject);

  std::vector<VkBuffer> &uniformBuffers = EngineData::i()->vkInstWrapper.uniformBuffers;
  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  std::vector<VkDeviceMemory> &uniformBufferMemory = EngineData::i()->vkInstWrapper.uniformBufferMemory;
  uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);

  std::vector<void *> &uniformBuffersMapped = EngineData::i()->vkInstWrapper.uniformBuffersMapped;
  uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    Buffer::createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         uniformBuffers[i], uniformBufferMemory[i]);
    vkMapMemory(EngineData::i()->vkInstWrapper.device, uniformBufferMemory[i], 0, size, 0, &uniformBuffersMapped[i]);
  }
}

void
Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propFlags, VkBuffer &buffer,
                     VkDeviceMemory &bufferMemory) {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
    LOG(F, "Could not create VertexBuffer");
  }

  VkMemoryRequirements memoryRequirements;
  vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memoryRequirements.size;
  allocInfo.memoryTypeIndex = SuitabilityChecker::findMemoryType(memoryRequirements.memoryTypeBits, propFlags);

  // Allocate the buffer memory TODO: Integrate VMA into this so we can use memoryOffset in vkBindBufferMemory
  if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
    LOG(F, "Could not allocate VkDeviceMemory for VkBuffer");
  }

  // Tells the GPU that the device memory corresponds to the buffer
  vkBindBufferMemory(device, buffer, bufferMemory, 0);
}


void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

  VkCommandBuffer singleUseCmdBuffer = Commandbuffer::recordSingleTime();

  VkBufferCopy copyRegion{};
  copyRegion.size = size;

  vkCmdCopyBuffer(singleUseCmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  Commandbuffer::endRecordSingleTime(singleUseCmdBuffer);
}

/**
 * Helper function to specify which part of the buffer is going to be
 * copied to the VkImage struct.
 */
void Buffer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer singleUseCmdBuffer = Commandbuffer::recordSingleTime();

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;

  region.imageOffset = {0, 0, 0};
  region.imageExtent = {
    width,
    height,
    1
  };

  vkCmdCopyBufferToImage(
    singleUseCmdBuffer,
    buffer,
    image,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1,
    &region
  );

  Commandbuffer::endRecordSingleTime(singleUseCmdBuffer);
}
