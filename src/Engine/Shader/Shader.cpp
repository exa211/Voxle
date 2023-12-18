#include "Shader.h"

#include <Engine.h>

/**
 *  @brief Loads a combined shader assuming they have the same name with .vert/.frag after the name
 **/
void VulkanShader::Shader::loadCombined(const std::string& name) {
  this->shaderName = name;

  std::string vertShaderPath = VOXLE_ROOT + std::string("/res/shader/compiled/" + name + ".vert.spv");
  std::string fragShaderPath = VOXLE_ROOT + std::string("/res/shader/compiled/" + name + ".frag.spv");

  this->vertShaderModule = ShaderUtil::Module::createShaderModule(ShaderUtil::Loading::readShaderFile(vertShaderPath));
  this->fragShaderModule = ShaderUtil::Module::createShaderModule(ShaderUtil::Loading::readShaderFile(fragShaderPath));
}

/**
 *  @brief Loads a combined shader assuming they have the same name with .vert/.frag after the name
 *
 *  @return Array of VkPipelineShaderStageCreateInfos for the pipeline creation.
 **/
std::array<VkPipelineShaderStageCreateInfo, 2> VulkanShader::Shader::getCreateInfo() {
  if(vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
    LOG(W, "Could not create VkPipelineShaderStageCreateInfo for shader: \n" + shaderName);
  }

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

  return {vertShaderStageInfo, fragShaderStageInfo};
}

void VulkanShader::Shader::destroy() {
  assert(EngineData::i()->vkInstWrapper.device != VK_NULL_HANDLE);
  vkDestroyShaderModule(EngineData::i()->vkInstWrapper.device, vertShaderModule, nullptr);
  vkDestroyShaderModule(EngineData::i()->vkInstWrapper.device, fragShaderModule, nullptr);
}

VkShaderModule ShaderUtil::Module::createShaderModule(const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule{};
  if(vkCreateShaderModule(EngineData::i()->vkInstWrapper.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    LOG(F, "Failed to create Shader Module");

  return shaderModule;
}
