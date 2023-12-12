#pragma once

#include <VulkanPipeline/Pipeline/Buffer/Buffer.h>
#include <deque>

#include "Block.hpp"

struct ChunkMesh {
  VertexBuffer vertexBuffer;
  IndexBuffer indexBuffer;
};

class Chunk {
  std::deque<Material> blocks;
public:
  std::string getBlock(int x, int y, int z);

  void generate();
  void regenerateMesh();
};
