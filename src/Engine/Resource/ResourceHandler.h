#pragma once

#include "Image/Image.h"
#include "stb_image.h"
#include <string>
#include <vector>

class Resources {
  // TODO: Texture Handling
public:
  static void createTexture(VulkanImage::Image &t, const std::string& path);
  static void createTextureArray(std::vector<VulkanImage::Image> images);
private:
  inline static const std::string TEXTURE_PATH = VOXLE_ROOT + std::string("/res/texture/");
};