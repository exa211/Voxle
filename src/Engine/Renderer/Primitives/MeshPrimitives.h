#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 col;
  glm::vec2 uv;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDesc{};

    // Vertex Position
    attributeDesc[0].binding = 0;
    attributeDesc[0].location = 0;
    attributeDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[0].offset = offsetof(Vertex, pos);

    // Vertex Color
    attributeDesc[1].binding = 0;
    attributeDesc[1].location = 1;
    attributeDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[1].offset = offsetof(Vertex, col);

    // Texture Coords | UV's
    attributeDesc[2].binding = 0;
    attributeDesc[2].location = 2;
    attributeDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDesc[2].offset = offsetof(Vertex, uv);
    return attributeDesc;
  }
};

struct BlockVertex {
  unsigned int packedVert;


  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(BlockVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static VkVertexInputAttributeDescription getAttributeDescriptions() {
    VkVertexInputAttributeDescription attrDesc{};

    // Packed values Vertex Pos, Index, TexID (wip)
    attrDesc.binding = 0;
    attrDesc.location = 0;
    attrDesc.format = VK_FORMAT_R32_UINT;
    attrDesc.offset = offsetof(BlockVertex, packedVert);
    return attrDesc;
  }
};