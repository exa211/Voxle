#include "Commandbuffer.h"

void Commandbuffer::create() {
  E_Data::i()->vkInstWrapper.commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = E_Data::i()->vkInstWrapper.commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t) E_Data::i()->vkInstWrapper.commandBuffers.size();

  if(vkAllocateCommandBuffers(E_Data::i()->vkInstWrapper.device, &allocInfo, E_Data::i()->vkInstWrapper.commandBuffers.data()) != VK_SUCCESS)
    LOG::fatal("Could not create VKCommandBuffer");
  LOG::info("Created VkCommandBuffer");

  // IMMEDIATE UPLOAD COMMAND BUFFER

  VkCommandBufferAllocateInfo allocInfoUpload{};
  allocInfoUpload.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfoUpload.commandPool = E_Data::i()->vkInstWrapper.immediateUploadPool;
  allocInfoUpload.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfoUpload.commandBufferCount = 1;

  if(vkAllocateCommandBuffers(E_Data::i()->vkInstWrapper.device, &allocInfoUpload, &E_Data::i()->vkInstWrapper.immediateCommandBuffer) != VK_SUCCESS)
    LOG::fatal("Could not create VKCommandBuffer");
  LOG::info("Created VkCommandBuffer for immediate uploads");
}

VkCommandBuffer Commandbuffer::recordSingleTime() {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = E_Data::i()->vkInstWrapper.commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(E_Data::i()->vkInstWrapper.device, &allocInfo, &commandBuffer);

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

  vkQueueSubmit(E_Data::i()->vkInstWrapper.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(E_Data::i()->vkInstWrapper.graphicsQueue);
  vkFreeCommandBuffers(E_Data::i()->vkInstWrapper.device, E_Data::i()->vkInstWrapper.commandPool, 1, &cmdBuffer);
}