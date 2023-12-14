#include "PrimitiveRenderer.h"

void PrimitiveRenderer::render(Camera& cam) {
  uint32_t currentFrame = 0;

  VkDevice &device = E_Data::i()->vkInstWrapper.device;
  VkSwapchainKHR &swapChain = E_Data::i()->vkInstWrapper.swapChain;

  VkFence& fence = E_Data::i()->vkInstWrapper.inFlightFences[currentFrame];
  VkCommandBuffer& cBuffer = E_Data::i()->vkInstWrapper.commandBuffers[currentFrame];

  // "Vulkan Mutex" - Semaphores are like signals that can signal each other, they block each other for synchronization
  VkSemaphore& imageSema = E_Data::i()->vkInstWrapper.imageAvailableSemas[currentFrame];
  VkSemaphore& renderSema = E_Data::i()->vkInstWrapper.renderFinishedSemas[currentFrame];

  vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;

  // Acquire next image from swapchain and Signal the Semaphore to block
  VkResult ifErr = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageSema, VK_NULL_HANDLE, &imageIndex);
  if(ifErr == VK_ERROR_OUT_OF_DATE_KHR) {
    VkSetup::recreateSwapchain(device);
    return;
  } else if(ifErr != VK_SUCCESS && ifErr != VK_SUBOPTIMAL_KHR) LOG(F, "Could not acquire VkSwapChainImage in Render[PrimitiveRenderer]");

  vkResetFences(device, 1, &fence);

  vkResetCommandBuffer(cBuffer, 0); // Reset the command buffer to make sure it's not filled

  /*
   *  RECORD COMMAND BUFFER
   **/

  Pipeline::recordCommandBuffer(cam, cBuffer, imageIndex);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitOnSemaphores[] = {imageSema};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitOnSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cBuffer;

  VkSemaphore signalForSemaphores[] = {renderSema};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalForSemaphores;

  if(vkQueueSubmit(E_Data::i()->vkInstWrapper.graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    LOG(F, "Could not submit command buffer, stopping renderer");

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalForSemaphores;

  VkSwapchainKHR swapChains[] = {swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;
  presentInfo.pImageIndices = &imageIndex;
  presentInfo.pResults = nullptr;

  // Present the next image to the window
  ifErr = vkQueuePresentKHR(E_Data::i()->vkInstWrapper.presentQueue, &presentInfo);
  if(ifErr == VK_ERROR_OUT_OF_DATE_KHR || ifErr == VK_SUBOPTIMAL_KHR || E_Data::i()->vkInstWrapper.framebufferWasResized) {
    E_Data::i()->vkInstWrapper.framebufferWasResized = false;
    VkSetup::recreateSwapchain(device);
    return;
  } else if(ifErr != VK_SUCCESS) LOG(F, "Could not acquire VkSwapChainImage in Render[PrimitiveRenderer]");

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

}

void PrimitiveRenderer::updateUniformBuffers(uint32_t currentFrame, Camera &cam) {
  UniformBufferObject ubo{};
  ubo.col = glm::vec3(1.0f, 1.0f, 1.0f);
  memcpy(E_Data::i()->vkInstWrapper.uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}