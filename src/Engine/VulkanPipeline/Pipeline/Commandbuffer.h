#pragma once

#include "Engine.h"

#include <glad/vulkan.h>


namespace Commandbuffer {
  void create();
  VkCommandBuffer recordSingleTime();
  void endRecordSingleTime(VkCommandBuffer cmdBuffer);
}
