#pragma once

#include <glad/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

struct Vertex {
  glm::vec3 pos;
  glm::vec3 col;

  static VkVertexInputBindingDescription getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
  }

  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDesc{};
    attributeDesc[0].binding = 0;
    attributeDesc[0].location = 0;
    attributeDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[0].offset = offsetof(Vertex, pos);

    attributeDesc[1].binding = 0;
    attributeDesc[1].location = 1;
    attributeDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDesc[1].offset = offsetof(Vertex, col);
    return attributeDesc;
  }
};

const std::vector<uint32_t> frontVerts = {
  0, 1, 1,
  1, 1, 1,
  1, 1, 0,
  0, 1, 0
};

const std::vector<uint32_t> backVerts = {
  0, 0, 0,
  1, 0, 0,
  1, 0, 1,
  0, 0, 1
};

const std::vector<uint32_t> botVerts = {
  1, 0, 0,
  0, 0, 0,
  0, 1, 0,
  1, 1, 0
};

const std::vector<uint32_t> topVerts = {
  0, 0, 1,
  1, 0, 1,
  1, 1, 1,
  0, 1, 1
};

const std::vector<uint32_t> rightVerts = {
  0, 0, 0,
  0, 0, 1,
  0, 1, 1,
  0, 1, 0
};
const std::vector<uint32_t> leftVerts = {
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  1, 1, 1
};

const std::vector<uint16_t> faceIndices = {
  0, 1, 2, 2, 3, 0
};