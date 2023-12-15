#pragma once

#include <vulkan/vulkan.h>
#include <Logging/Logger.h>
#include "VulkanPipeline/Suitability/SuitabilityChecker.h"
#include "Renderer/Primitives/MeshPrimitives.h"

#include <vk_mem_alloc.h>

namespace Buffers {

  struct VmaBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
  };

  struct IndexBuffer {
    VkBuffer indexBuffer;
    VmaAllocation allocation;
    uint32_t indicesSize;
  };

  struct UniformBufferObject {
    alignas(16) glm::mat4 model{1};
    alignas(16)glm::mat4 view;
    alignas(16)glm::mat4 proj;
    alignas(16) glm::vec3 col;
  };

  VmaBuffer createVertexBuffer(const std::vector<Vertex>& vertices);
  VmaBuffer createBlockVertexBuffer(const std::vector<BlockVertex>& blockVertices);
  IndexBuffer createIndexBuffer(const std::vector<uint32_t>& indices);
  void createUniformBuffers();
  void createBufferVMA(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propFlags, VkBuffer& buffer, VmaAllocation &allocation);
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propFlags, VkBuffer& buffer, VkDeviceMemory &deviceMemory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
}