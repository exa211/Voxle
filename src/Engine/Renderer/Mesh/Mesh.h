#pragma once

#include <vulkan/vulkan.h>
#include "VulkanPipeline/Pipeline/Buffer/Buffer.h"
#include "VulkanPipeline/Pipeline/PushConstants/GenericPushConstants.h"

class Mesh {
public:
  VertexBuffer vertexBuffer;
  IndexBuffer indexBuffer;
  MeshPushConstant meshRenderData;
};
