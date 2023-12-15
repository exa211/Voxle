#include "GraphicsPipeline.h"
#include "PushConstants/GenericPushConstants.h"
#include "Renderer/PrimitiveRenderer.h"
#include "VulkanPipeline/Suitability/SuitabilityChecker.h"

void Pipeline::createGraphicsPipeline(VkDescriptorSetLayout &descriptor) {

  auto vertShaderCode = Shader::Loading::readShaderFile("../res/shader/compiled/vert.spv");
  auto fragShaderCode = Shader::Loading::readShaderFile("../res/shader/compiled/frag.spv");

  VkShaderModule vertShaderModule = Shader::Module::createShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = Shader::Module::createShaderModule(fragShaderCode);

  // Graphics Pipeline Creation

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; // Tells what stage should be created
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main"; // The main function of the shader code

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // Tells what stage should be created
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main"; // The main function of the shader code

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

  std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
  dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

  VkPipelineVertexInputStateCreateInfo vertInputCreateInfo{};
  vertInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto vertexBindingDescription = BlockVertex::getBindingDescription();
  auto vertexAttrDescriptions = BlockVertex::getAttributeDescriptions();

  vertInputCreateInfo.vertexBindingDescriptionCount = 1;
  //vertInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttrDescriptions.size());

  vertInputCreateInfo.vertexAttributeDescriptionCount = 1;

  vertInputCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
  vertInputCreateInfo.pVertexAttributeDescriptions = &vertexAttrDescriptions;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = ((float) EngineData::i()->vkInstWrapper.extent.height);
  viewport.width = (float) EngineData::i()->vkInstWrapper.extent.width;
  viewport.height = -((float) EngineData::i()->vkInstWrapper.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = EngineData::i()->vkInstWrapper.extent;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  // RASTERIZER SETTINGS

  VkPipelineRasterizationStateCreateInfo rasterCreateInfo{};
  rasterCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterCreateInfo.depthClampEnable = VK_FALSE;
  rasterCreateInfo.rasterizerDiscardEnable = VK_FALSE;
  rasterCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; // Change this for line, or point drawing
  rasterCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterCreateInfo.depthBiasEnable = VK_FALSE;
  rasterCreateInfo.depthBiasConstantFactor = 0.0f;
  rasterCreateInfo.depthBiasClamp = 0.0f;
  rasterCreateInfo.depthBiasSlopeFactor = 0.0f;
  rasterCreateInfo.lineWidth = 1.0f;

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

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachmentState;

  VkPipelineDepthStencilStateCreateInfo depthTest{};
  depthTest.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthTest.depthTestEnable = VK_TRUE;
  depthTest.depthWriteEnable = VK_TRUE;
  depthTest.depthCompareOp = VK_COMPARE_OP_LESS;
  depthTest.depthBoundsTestEnable = VK_FALSE; // <- We don't need depth clipping
  // We don't need stencil testing at the moment.
  // Will come in handy later if we have boats or something where geometry is below water.
  depthTest.stencilTestEnable = VK_FALSE;

  // Pipeline Layout

  VkPipelineLayout layout{};
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

  VkPushConstantRange transformPushConstant;
  transformPushConstant.offset = 0;
  transformPushConstant.size = sizeof(MeshPushConstant);
  transformPushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptor;
  // Push Constants
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &transformPushConstant;

  if (vkCreatePipelineLayout(EngineData::i()->vkInstWrapper.device, &pipelineLayoutInfo, nullptr, &layout) !=
      VK_SUCCESS)
    LOG(F, "Could not create VkPipelineLayout in GraphicsPipeline.cpp");

  EngineData::i()->vkInstWrapper.pipelineLayout = layout;
  LOG(I, "Created VkPipelineLayout");

  if (EngineData::i()->vkInstWrapper.renderPass == nullptr) LOG(F, "VkRenderPass is null, exiting");

  // Create Graphics Pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertInputCreateInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterCreateInfo;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthTest; // <- Depth testing / Depth stencil
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicStateCreateInfo;

  pipelineInfo.layout = layout;
  pipelineInfo.renderPass = EngineData::i()->vkInstWrapper.renderPass;
  pipelineInfo.subpass = 0;

  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  LOG(I, "Creating Pipeline...");

  if (vkCreateGraphicsPipelines(EngineData::i()->vkInstWrapper.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                &EngineData::i()->vkInstWrapper.pipeline) != VK_SUCCESS)
    LOG(F, "Could not create VkPipeline in GraphicsPipeline.cpp");
  LOG(I, "Created VkPipeline \n\tGraphics Pipeline Creation finished.");

  vkDestroyShaderModule(EngineData::i()->vkInstWrapper.device, vertShaderModule, nullptr);
  vkDestroyShaderModule(EngineData::i()->vkInstWrapper.device, fragShaderModule, nullptr);
  LOG(W, "Cleaning up shaders");
}

void Pipeline::createFramebuffers() {
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
void Pipeline::createCommandPool() {
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
void Pipeline::createDepthBufferingObjects() {
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

void Pipeline::recordCommandBuffer(Camera &cam, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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
  clearValues[0].color = {{0.2f, 0.2f, 0.2f}};
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

  glm::mat4 view = cam.cameraMatrix.view;
  glm::mat4 proj = cam.cameraMatrix.proj;

  // ---- Render meshes ----

  // Bind the Pipeline to the commandBuffer
  // Don't need to bind this for every single mesh
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, EngineData::i()->vkInstWrapper.pipeline);

  for (Mesh &m: SceneManager::i()->curScene.meshesInScene) {

    // TODO: MATRIXS CALCULATIONS ON CPU IS NOT REALLY FAST CONSIDER JUST GIVING THE GPU/SHADER ROTATION, TRANSLATION AND SCALE VALUES

    glm::mat4 model = m.meshRenderData.transformMatrix;
    glm::vec4 color = m.meshRenderData.data;
    MeshPushConstant constant{};
    constant.transformMatrix = proj * view * model;
    constant.data = color;
    // Push minimal transformation data to shader
    vkCmdPushConstants(commandBuffer, EngineData::i()->vkInstWrapper.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(MeshPushConstant), &constant);

    // Bind the mesh if it's not the same
    if (&m != lastRenderedMesh) {

      VkDeviceSize offsets = 0;
      // Bind VBO
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m.vertexBuffer.buffer, &offsets);
      // Bind IBO
      vkCmdBindIndexBuffer(commandBuffer, m.indexBuffer.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
      lastRenderedMesh = &m;
    }
    // Bind UBO
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vki.pipelineLayout, 0, 1,
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

void Pipeline::createSyncObjects() {
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

  if (vkCreateFence(device, &uploadFenceInfo, nullptr, &EngineData::i()->vkInstWrapper.immediateUploadFence) != VK_SUCCESS) {
    LOG(F, "Could not create VkFence for immediate upload");
  }
  LOG(I, "Created VkFence for immediate upload");
}

void Pipeline::immediateSubmit(std::function<void(VkCommandBuffer)> &&function) {
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

