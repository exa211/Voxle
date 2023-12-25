#include "GraphicsPipeline.h"
#include "PushConstants/GenericPushConstants.h"
#include "VulkanPipeline/Suitability/SuitabilityChecker.h"

#include "Scene/SceneManager.h"

#include "Engine.h"
#include "Util/ColorUtil.hpp"

void VulkanPipeline::createFramebuffers() {
  std::vector<VkFramebuffer> buffers = EngineData::i()->vkInstWrapper.swapChainFramebuffers;
  buffers.resize(EngineData::i()->vkInstWrapper.swapChainImageViews.size());

  std::vector<VkImageView> &imageViews = EngineData::i()->vkInstWrapper.swapChainImageViews;

  VkImageView depthView = EngineData::i()->vkInstWrapper.depthImage.vkImageView;

  int frameBufferCount = 0;
  for (size_t i = 0; i < imageViews.size(); i++) {
    std::array<VkImageView, 2> attachments = {imageViews[i], depthView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = EngineData::i()->vkInstWrapper.renderPass;
    framebufferInfo.attachmentCount = static_cast<int>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = EngineData::i()->vkInstWrapper.extent.width;
    framebufferInfo.height = EngineData::i()->vkInstWrapper.extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(EngineData::i()->vkInstWrapper.device, &framebufferInfo, nullptr, &buffers[i]) !=
        VK_SUCCESS)
      LOG(F, "Error while creating Framebuffers");
    ++frameBufferCount;
  }
  LOG(I, "Created " + std::to_string(frameBufferCount) + " VkFrameBuffers");
  EngineData::i()->vkInstWrapper.swapChainFramebuffers = buffers;
}

/**
 *  Creates a general CommandPool for normal rendering and one for immediate uploading.
 *  STAGE: After PhysicalDevice, Device and Surface
 **/
void VulkanPipeline::createCommandPool() {
  QueueFamilyIndices queueFamilyIndices = QueueHelper::findQueueFamilies(EngineData::i()->vkInstWrapper.physicalDevice,
                                                                         EngineData::i()->vkInstWrapper.surface);
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
  if (vkCreateCommandPool(EngineData::i()->vkInstWrapper.device, &poolInfo, nullptr,
                          &EngineData::i()->vkInstWrapper.commandPool) != VK_SUCCESS)
    LOG(F, "Could not create VkCommandPool");
  LOG(I, "Created VkCommandPool");

  // IMMEDIATE UPLOAD POOL

  if (vkCreateCommandPool(EngineData::i()->vkInstWrapper.device, &poolInfo, nullptr,
                          &EngineData::i()->vkInstWrapper.immediateUploadPool) != VK_SUCCESS)
    LOG(F, "Could not create VkCommandPool");
  LOG(I, "Created VkCommandPool for immediate uploads");
}

/**
 *  Creates DepthBufferImage for buffering current scene depth.
 **/
void VulkanPipeline::createDepthBufferingObjects() {
  VkFormat depthFormat = SuitabilityChecker::getSupportedDepthFormat();

  int imgWidth = static_cast<int>(EngineData::i()->vkInstWrapper.extent.width);
  int imgHeight = static_cast<int>(EngineData::i()->vkInstWrapper.extent.height);

  // Create image
  VulkanImage::createImage(imgWidth, imgHeight, depthFormat,
                           VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                           EngineData::i()->vkInstWrapper.depthImage.vkImage,
                           EngineData::i()->vkInstWrapper.depthImage.vkImageMemory);

  VulkanImage::createImageView(EngineData::i()->vkInstWrapper.depthImage.vkImage,
                               EngineData::i()->vkInstWrapper.depthImage.vkImageView,
                               depthFormat,
                               VK_IMAGE_ASPECT_DEPTH_BIT);

  // Transition image
  VulkanImage::transitionImageLayout(EngineData::i()->vkInstWrapper.depthImage.vkImage,
                                     depthFormat,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void VulkanPipeline::recordCommandBuffer(Camera &cam, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
  VulkanInstance &vki = EngineData::i()->vkInstWrapper;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = 0;
  beginInfo.pInheritanceInfo = nullptr;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    LOG(F, "Could not start recording VkCommandBuffer");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = EngineData::i()->vkInstWrapper.renderPass;
  renderPassInfo.framebuffer = EngineData::i()->vkInstWrapper.swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = EngineData::i()->vkInstWrapper.extent;

  std::array<VkClearValue, 2> clearValues{};
  glm::vec3 cColor = ColorUtil::convertRGBtoFloat({169, 196, 201});
  clearValues[0].color = {{cColor.x, cColor.y, cColor.z}};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(EngineData::i()->vkInstWrapper.extent.width);
  viewport.height = static_cast<float>(EngineData::i()->vkInstWrapper.extent.height);
  viewport.minDepth = 0;
  viewport.maxDepth = 1;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = EngineData::i()->vkInstWrapper.extent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  // Begins the renderPass with the specified renderPassInfo
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  Mesh *lastRenderedMesh = nullptr;
  VulkanPipeline::Pipeline *lastUsedPipeline = nullptr;

  glm::mat4 view = cam.cameraMatrix.view;
  glm::mat4 proj = cam.cameraMatrix.proj;

  // ---- Render meshes ----

  // Bind the Pipeline to the commandBuffer
  // Don't need to bind this for every single mesh

  VulkanPipeline::Pipeline currentPipeline = EngineData::i()->vkInstWrapper.pipelines.find("global")->second;

  if(EngineData::i()->vkInstWrapper.currentShader == 1) {
    currentPipeline = EngineData::i()->vkInstWrapper.pipelines.find("debug")->second;
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.getPipeline());

  for (Mesh &m: SceneManager::i()->curScene.meshesInScene) {

    // Bind different pipeline if required
    if(m.shader != nullptr && lastUsedPipeline != m.shader) {
      vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m.shader->getPipeline());
      lastUsedPipeline = m.shader;
    }

    // TODO: MATRIXS CALCULATIONS ON CPU IS NOT REALLY FAST CONSIDER JUST GIVING THE GPU/SHADER ROTATION, TRANSLATION AND SCALE VALUES

    glm::mat4 model = m.meshRenderData.transformMatrix;
    glm::vec4 color = m.meshRenderData.data;
    MeshPushConstant constant{};
    constant.transformMatrix = proj * view * model;
    constant.data = color;
    // Push minimal transformation data to shader
    vkCmdPushConstants(commandBuffer, currentPipeline.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(MeshPushConstant), &constant);

    // Bind different Mesh if required
    if (&m != lastRenderedMesh) {

      VkDeviceSize offsets = 0;
      // Bind VBO
      if(m.vertexBuffer.buffer == VK_NULL_HANDLE) continue;
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m.vertexBuffer.buffer, &offsets);
      // Bind IBO
      vkCmdBindIndexBuffer(commandBuffer, m.indexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
      lastRenderedMesh = &m;
    }
    // Bind UBO
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentPipeline.getPipelineLayout(), 0, 1,
                            vki.descriptorSets.data(), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, m.indexBuffer.indicesSize, 1, 0, 0, 0);
  }

  // ---- Render meshes End ----

  // ImGUI Rendering TODO: Seperate into EngineUI class
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

  vkCmdEndRenderPass(commandBuffer);
  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) LOG(F, "Could not end VkCommandBuffer recording");

//  Mesh& m = SceneManager::i()->curScene.meshesInScene.at(1);
//
//  VkBuffer vertexBuffers[] = {m.vertexBuffer.buffer};
//  VkDeviceSize offsets[] = {0};
//  vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
//
//  vkCmdBindIndexBuffer(commandBuffer, m.indexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
//
//  // Bind uniform buffers
//  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vki.pipelineLayout, 0, 1, &vki.descriptorSets[imageIndex], 0, nullptr);
//
//  //vkCmdDraw(commandBuffer, vertCount, 1, 0, 0);
//  vkCmdDrawIndexed(commandBuffer, m.indexBuffer.size, 1, 0, 0, 0);

}

void VulkanPipeline::createSyncObjects() {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;

  EngineData::i()->vkInstWrapper.imageAvailableSemas.resize(MAX_FRAMES_IN_FLIGHT);
  EngineData::i()->vkInstWrapper.renderFinishedSemas.resize(MAX_FRAMES_IN_FLIGHT);
  EngineData::i()->vkInstWrapper.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &EngineData::i()->vkInstWrapper.imageAvailableSemas[i])
        || vkCreateSemaphore(device, &semaphoreInfo, nullptr, &EngineData::i()->vkInstWrapper.renderFinishedSemas[i])
        || vkCreateFence(device, &fenceInfo, nullptr, &EngineData::i()->vkInstWrapper.inFlightFences[i])) {
      LOG(F, "Could not create synchronization objects (VkSemaphore and VkFence)");
    }
  }
  LOG(I, "Created synchronization objects (VkSemaphore and VkFence)");

  // IMMEDIATE UPLOAD FENCE

  VkFenceCreateInfo uploadFenceInfo{};
  uploadFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if (vkCreateFence(device, &uploadFenceInfo, nullptr, &EngineData::i()->vkInstWrapper.immediateUploadFence) !=
      VK_SUCCESS) {
    LOG(F, "Could not create VkFence for immediate upload");
  }
  LOG(I, "Created VkFence for immediate upload");
}

void VulkanPipeline::immediateSubmit(std::function<void(VkCommandBuffer)> &&function) {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;
  VkFence &fence = EngineData::i()->vkInstWrapper.immediateUploadFence;

  VkCommandBuffer &cmd = EngineData::i()->vkInstWrapper.immediateCommandBuffer;

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  beginInfo.pInheritanceInfo = nullptr;
  if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
    LOG(F, "Could not one-time submit VkCommandBuffer");

  function(cmd); // Execute the given function with the commmand buffer

  vkEndCommandBuffer(cmd);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.waitSemaphoreCount = 0;
  submitInfo.pWaitSemaphores = nullptr;
  submitInfo.pWaitDstStageMask = nullptr;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;
  submitInfo.signalSemaphoreCount = 0;
  submitInfo.pSignalSemaphores = nullptr;

  if (vkQueueSubmit(EngineData::i()->vkInstWrapper.graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    LOG(F, "Could not Queue Submit one-time command");

  vkWaitForFences(device, 1, &fence, true, 9999);
  vkResetFences(device, 1, &fence);

  vkResetCommandPool(device, EngineData::i()->vkInstWrapper.immediateUploadPool, 0);
}

/**
 *  @brief Loads a shader (frag and vert) from the given path and sets it for pipeline creation.
 **/
void VulkanPipeline::Pipeline::bindShader(const std::string &sName) {
  VulkanShader::Shader t_shader;
  t_shader.loadCombined(sName);
  this->shader = t_shader;
}

/**
 *  @brief Tells the pipeline about our attributes on our vertices like color or position.
 **/
void VulkanPipeline::Pipeline::setVertexDescriptions(VkVertexInputBindingDescription desc,
                                                     std::vector<VkVertexInputAttributeDescription> attrDescriptions) {

  vertInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  vertInputCreateInfo.vertexBindingDescriptionCount = 1;
  vertInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescriptions.size());

  vertInputCreateInfo.pVertexBindingDescriptions = &desc;
  vertInputCreateInfo.pVertexAttributeDescriptions = attrDescriptions.data();

}

void VulkanPipeline::Pipeline::setInputAssembly(VkPrimitiveTopology topology) {
  vertInputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  vertInputAssemblyCreateInfo.topology = topology;
  vertInputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
}

void VulkanPipeline::Pipeline::setDescriptorLayout(VkDescriptorSetLayout &inLayout) {
  this->descSetLayout = inLayout;
}

void VulkanPipeline::Pipeline::setRenderPass(VkRenderPass inRenderPass) {
  this->renderpass = inRenderPass;
}

void VulkanPipeline::Pipeline::build() {
  VkDevice &device = EngineData::i()->vkInstWrapper.device;

  // >- VkPipelineVertexInputStateCreateInfo -<

  //VkPipelineVertexInputAssemblyStateCreateInfo
  setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

  // Viewport
  viewport.x = 0.0f;
  viewport.y = ((float) EngineData::i()->vkInstWrapper.extent.height);
  viewport.width = (float) EngineData::i()->vkInstWrapper.extent.width;
  viewport.height = -((float) EngineData::i()->vkInstWrapper.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  //Scissor
  scissor.offset = {0, 0};
  scissor.extent = EngineData::i()->vkInstWrapper.extent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  // >- RASTERIZER -<

  rasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterCreateInfo.depthClampEnable = VK_FALSE;
  rasterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterCreateInfo.polygonMode = polygonMode; // Change this for line, or point drawing
  rasterCreateInfo.cullMode = cullMode;
  rasterCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterCreateInfo.depthBiasEnable = VK_FALSE;
  rasterCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterCreateInfo.depthBiasClamp = 0.0f;
  rasterCreateInfo.depthBiasSlopeFactor = 0.0f;
  rasterCreateInfo.lineWidth = 1.0f;

  // >- MULTISAMPLING -<

  // I think this explains itself if you dont know what multisampling is in ${CURRENT_YEAR}
  // you are in the wrong universe, well maybe in your universe there's no MSAA??
  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // Color blend state see transparency or additive color merging
  VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
  colorBlendAttachmentState.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachmentState.blendEnable = VK_FALSE;
  colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

  // >- COLOR BLENDING -<

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachmentState;

  // >- DEPTH TESTING -<

  VkPipelineDepthStencilStateCreateInfo depthTest{};
  depthTest.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthTest.depthTestEnable = VK_TRUE;
  depthTest.depthWriteEnable = VK_TRUE;
  depthTest.depthCompareOp = VK_COMPARE_OP_LESS;
  depthTest.depthBoundsTestEnable = VK_FALSE; // <- We don't need depth clipping
  // We don't need stencil testing at the moment.
  // Will come in handy later if we have boats or something where geometry is below water.
  depthTest.stencilTestEnable = VK_FALSE;

  // >- LAYOUT -<

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

  VkPushConstantRange transformPushConstant;
  transformPushConstant.offset = 0;
  transformPushConstant.size = sizeof(MeshPushConstant);
  transformPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descSetLayout;
  // Push Constants
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &transformPushConstant;

  assert(device != VK_NULL_HANDLE);

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout)) {
    LOG(F, "Could not create GraphicsPipeline because PipelineLayout creation failed.");
  }

  assert(renderpass != VK_NULL_HANDLE);

  VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shader.getCreateInfo().data();
  pipelineCreateInfo.pVertexInputState = &vertInputCreateInfo;
  pipelineCreateInfo.pInputAssemblyState = &vertInputAssemblyCreateInfo;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pRasterizationState = &rasterCreateInfo;
  pipelineCreateInfo.pMultisampleState = &multisampling;
  pipelineCreateInfo.pDepthStencilState = &depthTest; // <- Depth testing / Depth stencil
  pipelineCreateInfo.pColorBlendState = &colorBlending;
  pipelineCreateInfo.pDynamicState = nullptr; // <- We don't want to use dynamic states anymore

  pipelineCreateInfo.layout = pipelineLayout;
  pipelineCreateInfo.renderPass = renderpass;
  pipelineCreateInfo.subpass = 0;

  pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineCreateInfo.basePipelineIndex = -1;

  LOG(I, "Creating Pipeline: " + pipelineName);

  if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &pipeline)) {
    LOG(F, "Could not create GraphicsPipeline.");
  }

  // Insert pipeline into pipelines unordered_map
  LOG(I, "Inserting Pipeline: " + pipelineName + " into pipeline map");
  EngineData::i()->vkInstWrapper.pipelines.insert({pipelineName, *this});

  LOG(I, "Created Pipeline.");
  LOG(D, "Cleaning up shader's");
  shader.destroy();
}

VkPipelineLayout &VulkanPipeline::Pipeline::getPipelineLayout() {
  return this->pipelineLayout;
}

VkPipeline &VulkanPipeline::Pipeline::getPipeline() {
  return this->pipeline;
}

void VulkanPipeline::Pipeline::setPolygonMode(VkPolygonMode inPolygonMode) {
  this->polygonMode = inPolygonMode;
}

void VulkanPipeline::Pipeline::setCulling(VkCullModeFlags inCullMode) {
  this->cullMode = inCullMode;
}

