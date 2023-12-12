#pragma once

#ifndef RESOURCEHANDLER_H
#define RESOURCEHANDLER_H

#include "Texture/Texture.h"
#include "stb_image.h"
#include <string>

namespace Resource {
  void loadTexture(Texture &t, const std::string& path);
}

#endif
