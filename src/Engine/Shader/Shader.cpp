#include "Shader.h"

VkShaderModule Shader::Module::createShaderModule(const std::vector<char> &code) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule{};
  if(vkCreateShaderModule(E_Data::i()->vkInstWrapper.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    LOG::fatal("Failed to create Shader Module");

  return shaderModule;
}
