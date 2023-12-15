#include "Commandbuffer.h"

void Commandbuffer::create() {
  EngineData::i()->vkInstWrapper.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = EngineData::i()->vkInstWrapper.commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t) EngineData::i()->vkInstWrapper.commandBuffers.size();

  if(vkAllocateCommandBuffers(EngineData::i()->vkInstWrapper.device, &allocInfo, EngineData::i()->vkInstWrapper.commandBuffers.data()) != VK_SUCCESS) {
    LOG(F, "Could not create VKCommandBuffer");
  }
  LOG(I, "Created VkCommandBuffer");

  // IMMEDIATE UPLOAD COMMAND BUFFER

  VkCommandBufferAllocateInfo allocInfoUpload{};
  allocInfoUpload.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfoUpload.commandPool = EngineData::i()->vkInstWrapper.immediateUploadPool;
  allocInfoUpload.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfoUpload.commandBufferCount = 1;

  if(vkAllocateCommandBuffers(EngineData::i()->vkInstWrapper.device, &allocInfoUpload, &EngineData::i()->vkInstWrapper.immediateCommandBuffer) != VK_SUCCESS) {
    LOG(F, "Could not create VKCommandBuffer");
  }
  LOG(I, "Created VkCommandBuffer for immediate uploads");
}

VkCommandBuffer Commandbuffer::recordSingleTime() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = EngineData::i()->vkInstWrapper.commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(EngineData::i()->vkInstWrapper.device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);
  return commandBuffer;
}

void Commandbuffer::endRecordSingleTime(VkCommandBuffer cmdBuffer) {
  vkEndCommandBuffer(cmdBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmdBuffer;

  vkQueueSubmit(EngineData::i()->vkInstWrapper.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(EngineData::i()->vkInstWrapper.graphicsQueue);
  vkFreeCommandBuffers(EngineData::i()->vkInstWrapper.device, EngineData::i()->vkInstWrapper.commandPool, 1, &cmdBuffer);
}