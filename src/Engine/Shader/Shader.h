#pragma once

#include <fstream>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>

#include <Logging/Logger.h>

namespace VulkanShader {

  // TODO: Destructor for cleaning up modules and code
  class Shader {
  public:
    void loadCombined(const std::string& filename);
    void destroy();
    std::vector<VkPipelineShaderStageCreateInfo> getCreateInfo();
  private:
    std::string shaderName{"undefined"};
    VkShaderModule vertShaderModule{};
    VkShaderModule fragShaderModule{};
  };

}

namespace ShaderUtil {

    namespace Loading {
      static std::vector<char> readShaderFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary); // Binary because SPIR-V is binary

        LOG(F, !file.is_open(), "Failed to open shader File: " + filename);

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        LOG(I, "Shader " << filename << " is " << buffer.size() << " bytes.");

        file.close();
        return buffer;
      }
    }

    namespace Module {
      VkShaderModule createShaderModule(const std::vector<char>& code);
    }

  }