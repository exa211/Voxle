#pragma once

#include <vector>

const std::vector<const char*> validationLayers = {
  "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidation = false;
#else
const bool enableValidation = true;
#endif

namespace VulkanValidation {

  bool checkValidationLayerSupport();
  std::vector<const char*> getRequireExtensions();

}