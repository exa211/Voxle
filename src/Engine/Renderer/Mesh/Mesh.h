#pragma once

#include <vulkan/vulkan.h>
#include "VulkanPipeline/Pipeline/Buffer/Buffer.h"
#include "VulkanPipeline/Pipeline/PushConstants/GenericPushConstants.h"
#include "GraphicsPipeline.h"

class Mesh {
public:
  Buffers::VmaBuffer vertexBuffer{};
  Buffers::IndexBuffer indexBuffer{};
  MeshPushConstant meshRenderData{};

  size_t vertexCount{0};

  VulkanPipeline::Pipeline* shader{};

  void destroy();
};
