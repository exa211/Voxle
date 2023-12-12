#pragma once

#include "Engine.h"

#include <vulkan/vulkan.h>


namespace Commandbuffer {
  void create();
  VkCommandBuffer recordSingleTime();
  void endRecordSingleTime(VkCommandBuffer cmdBuffer);
}
