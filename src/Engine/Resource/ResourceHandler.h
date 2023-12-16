#pragma once

#include "Image/Image.h"
#include "stb_image.h"
#include <string>

class Resources {
  // TODO: Texture Handling
public:
  static void createTexture(VulkanImage::Image &t, const std::string& path);
private:
  inline static const std::string TEXTURE_PATH = VOXLE_ROOT + std::string("/res/texture/");
};