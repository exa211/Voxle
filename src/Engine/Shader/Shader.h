#pragma once

#include <fstream>
#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#include <Engine.h>

#include <Logging/Logger.h>

namespace Shader {

    namespace Loading {
      static std::vector<char> readShaderFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

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