#pragma once

#include "Image/Image.h"
#include "stb_image.h"
#include <string>

namespace Resources {
  // TODO: Texture Handling
  void createTexture(VulkanImage::Image &t, const std::string& path);
}
