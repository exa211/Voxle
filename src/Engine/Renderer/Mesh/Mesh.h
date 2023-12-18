#pragma once

#include <vulkan/vulkan.h>
#include "VulkanPipeline/Pipeline/Buffer/Buffer.h"
#include "VulkanPipeline/Pipeline/PushConstants/GenericPushConstants.h"

class Mesh {
public:
  Buffers::VmaBuffer vertexBuffer{};
  Buffers::IndexBuffer indexBuffer{};
  MeshPushConstant meshRenderData{};

  void destroy();
};
