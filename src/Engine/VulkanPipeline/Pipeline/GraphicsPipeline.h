#pragma once

#include <functional>
#include <utility>

#include "imgui_impl_vulkan.h"

#include "Shader/Shader.h"
#include "../Queue/QueueHelper.h"

#include "../VkSetup.h"
#include "Camera/Camera.h"

static struct GraphicsPipelineStages {
  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags dstStage;
} graphicsPipeline;

namespace VulkanPipeline {

  class Pipeline {
  public:
    explicit Pipeline(std::string  pipelineName) : pipelineName(std::move(pipelineName)) {}

    void build();
    void bindShader(const std::string& sName);
    void setVertexDescriptions(VkVertexInputBindingDescription desc,
                          std::vector<VkVertexInputAttributeDescription> attrDescriptions);

    void setInputAssembly(VkPrimitiveTopology topology);
    void setDescriptorLayout(VkDescriptorSetLayout& layout);
    void setRenderPass(VkRenderPass inRenderPass);
    void setPolygonMode(VkPolygonMode inPolygonMode);

    // >- GETTER -<
    VkPipeline& getPipeline();
    VkPipelineLayout& getPipelineLayout();
  private:
    std::string pipelineName{"undefined"};
    VkPipeline pipeline{};
    // Create infos
    VkPipelineVertexInputStateCreateInfo vertInputCreateInfo{};
    VkPipelineInputAssemblyStateCreateInfo vertInputAssemblyCreateInfo{};
    VkPipelineRasterizationStateCreateInfo rasterCreateInfo{};

    VkDescriptorSetLayout descSetLayout{};
    VkPipelineLayout pipelineLayout{};

    VkViewport viewport{};
    VkRect2D scissor{};

    VkRenderPass renderpass{};

    VkPolygonMode polygonMode;

    VulkanShader::Shader shader{};
  };

  void createFramebuffers();

  void createCommandPool();

  void createDepthBufferingObjects();

  void recordCommandBuffer(Camera& cam, VkCommandBuffer commandBuffer, uint32_t imageIndex);
  void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

  void createSyncObjects();
}