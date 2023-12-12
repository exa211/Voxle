#pragma once

#include <functional>
#include "Shader/Shader.h"
#include "../Queue/QueueHelper.h"
#include "../Engine/Renderer/Primitives/MeshPrimitives.h"
#include "../Engine/Scene/SceneManager.h"
#include "imgui_impl_vulkan.h"
#include "../VkSetup.h"
#include "Camera/Camera.h"

static struct GraphicsPipeline {
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags dstStage;
} graphicsPipeline;

namespace Pipeline {

  void createGraphicsPipeline(VkDescriptorSetLayout& descriptor);

  void createFramebuffers();

  void createCommandPool();

  void createDepthBufferingObjects();

  void recordCommandBuffer(Camera& cam, VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

  void createSyncObjects();
}