#include "Mesh.h"

#include "Engine.h"

void Mesh::destroy() {
  VmaAllocator &allocator = EngineData::i()->vkInstWrapper.vmaAllocator;
  vmaDestroyBuffer(allocator, vertexBuffer.buffer, vertexBuffer.allocation);
  vmaDestroyBuffer(allocator, indexBuffer.indexBuffer, indexBuffer.allocation);
}