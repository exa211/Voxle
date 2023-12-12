#pragma once

#include <glad/vulkan.h>
#include <Engine.h>
#include <Logging/Logger.h>
#include "VulkanPipeline/Suitability/SuitabilityChecker.h"
#include "Renderer/Primitives/MeshPrimitives.h"
#include <VulkanPipeline/Pipeline/Commandbuffer.h>

struct VertexBuffer {
  VkBuffer buffer;
  VkDeviceMemory bufferMemory;
  VkDeviceSize size;
};

struct IndexBuffer {
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  VkDeviceSize size;
      uint32_t indicesSize;
};

struct UniformBufferObject {
  alignas(16) glm::mat4 model{1};
  alignas(16)glm::mat4 view;
  alignas(16)glm::mat4 proj;
  alignas(16) glm::vec3 col;
};

namespace Buffer {

  VertexBuffer createVertexBuffer(const std::vector<Vertex>& vertices);
  IndexBuffer createIndexBuffer(const std::vector<uint32_t>& indices);
  void createUniformBuffers();
  void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propFlags, VkBuffer& buffer, VkDeviceMemory& memory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
}