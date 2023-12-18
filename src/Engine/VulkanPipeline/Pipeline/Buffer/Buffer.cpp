#include "Buffer.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <VulkanPipeline/Pipeline/Commandbuffer.h>

//#define BUFFER_DEBUG

Buffers::VmaBuffer Buffers::createVertexBuffer(const std::vector<Vertex> &vertices) {
  VmaAllocator &allocator = EngineData::i()->vkInstWrapper.vmaAllocator;

  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

#ifdef BUFFER_DEBUG
  LOG(D, bufferSize);
#endif

  // Create the Staging VkBuffer
  // This is used to transfer the given vertex data into high performance memory from the graphics card
  VmaBuffer stagingBuffer{};
  createBufferVMA(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer,
                  stagingBuffer.allocation);

  // This is mapping the buffer memory to CPU accessible memory
  void *data;
  vmaMapMemory(allocator, stagingBuffer.allocation, &data);
  memcpy(data, vertices.data(), bufferSize);
  vmaUnmapMemory(allocator, stagingBuffer.allocation);

  // Construct our wrapper VertexBuffer structure
  VmaBuffer vb{};

  // Create the local device buffer on the gpu
  createBufferVMA(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb.buffer, vb.allocation);
  // Copy the vertex data to the dst local vertex buffer ()
  copyBuffer(stagingBuffer.buffer, vb.buffer, bufferSize);

  // Free the resources used by the staging buffer | VMA
  vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);

  return vb;
}

Buffers::VmaBuffer Buffers::createBlockVertexBuffer(const std::vector<BlockVertex> &blockVertices) {
  VmaAllocator &allocator = EngineData::i()->vkInstWrapper.vmaAllocator;
  VkDeviceSize bufferSize = sizeof(blockVertices[0]) * blockVertices.size();

#ifdef BUFFER_DEBUG
  LOG(D, bufferSize);
#endif

  // Create the Staging VkBuffer
  // This is used to transfer the given vertex data into high performance memory from the graphics card
  VmaBuffer stagingBuffer{};
  createBufferVMA(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer,
                  stagingBuffer.allocation);

#ifdef BUFFER_DEBUG
  LOG(D, "creating block vertex buffer after staging buffer");
#endif

  // This is mapping the buffer memory to CPU accessible memory
  void *data;
  vmaMapMemory(allocator, stagingBuffer.allocation, &data);
  memcpy(data, blockVertices.data(), bufferSize);
  vmaUnmapMemory(allocator, stagingBuffer.allocation);

  // Construct our wrapper VertexBuffer structure
  VmaBuffer vb{};

  // Create the local device buffer on the gpu
  createBufferVMA(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb.buffer, vb.allocation);
  // Copy the vertex data to the dst local vertex buffer ()
  copyBuffer(stagingBuffer.buffer, vb.buffer, bufferSize);

  // Free the resources used by the staging buffer
  vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);

  return vb;
}

Buffers::IndexBuffer Buffers::createIndexBuffer(const std::vector<uint32_t> &indices) {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;
  VmaAllocator &allocator = EngineData::i()->vkInstWrapper.vmaAllocator;

  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VmaBuffer stagingBuffer{};
  createBufferVMA(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer.buffer,
                  stagingBuffer.allocation);

  // This is mapping the buffer memory to CPU accessible memory
  void *data;
  vmaMapMemory(allocator, stagingBuffer.allocation, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vmaUnmapMemory(allocator, stagingBuffer.allocation);

  // Local index buffer
  IndexBuffer localBuffer{};
  localBuffer.indicesSize = indices.size();

  createBufferVMA(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, localBuffer.indexBuffer, localBuffer.allocation);
  copyBuffer(stagingBuffer.buffer, localBuffer.indexBuffer, bufferSize);

  vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);

  return localBuffer;
}

void Buffers::createUniformBuffers() {
  VkDeviceSize size = sizeof(UniformBufferObject);

  std::vector<VmaBuffer> &uniformBuffers = EngineData::i()->vkInstWrapper.uniformBuffers;
  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  std::vector<void *> &uniformBuffersMapped = EngineData::i()->vkInstWrapper.uniformBuffersMapped;
  uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    Buffers::createBufferVMA(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          uniformBuffers[i].buffer, uniformBuffers[i].allocation);
    vmaMapMemory(EngineData::i()->vkInstWrapper.vmaAllocator, uniformBuffers[i].allocation, &uniformBuffersMapped[i]);
  }
}

void Buffers::createBufferVMA(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propFlags, VkBuffer &buffer,
                              VmaAllocation &allocation) {

  if(size == 0) {
    LOG(W, "Buffer size cannot be 0");
    return;
  }

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  // VMA Allocator
  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  if(vmaCreateBuffer(EngineData::i()->vkInstWrapper.vmaAllocator, &bufferInfo, &vmaAllocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
    LOG(F, "Could not create Buffer");
  }
}

// Deprecated
void Buffers::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propFlags, VkBuffer &buffer,
                           VkDeviceMemory& deviceMemory) {
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

  // VMA Allocator
  VmaAllocationCreateInfo vmaAllocInfo{};
  vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memoryRequirements.size;
  allocInfo.memoryTypeIndex = SuitabilityChecker::findMemoryType(memoryRequirements.memoryTypeBits, propFlags);

  // Allocate the buffer memory TODO: Integrate VMA into this so we can use memoryOffset in vkBindBufferMemory
  if (vkAllocateMemory(device, &allocInfo, nullptr, &deviceMemory) != VK_SUCCESS) {
    LOG(F, "Could not allocate VkDeviceMemory for VkBuffer");
  }

  // Tells the GPU that the device memory corresponds to the buffer
  vkBindBufferMemory(device, buffer, deviceMemory, 0);
}

/**
 * @brief This copies the given srcBuffer to the dstBuffer (likely on the gpu)
 *        It should be noted that this invokes a single command buffer record and is currently not thread safe.
 * */
void Buffers::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
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
void Buffers::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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
