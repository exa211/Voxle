#pragma once

#include <VulkanPipeline/Pipeline/Buffer/Buffer.h>
#include <deque>

#include "Block.hpp"

#define CHUNK_SIZE 64

struct ChunkMesh {
  std::vector<BlockVertex> vertices;
  std::vector<uint32_t> indices;
};

class Chunk {
  std::deque<Material> blocks;
  ChunkMesh chunkMesh;
public:
  Material getBlockUnsafe(int x, int y, int z);
  Material& getBlock(int xSafe, int ySafe, int zSafe);
  ChunkMesh& getChunkMesh();

  void generate(std::vector<float>& noise);
  void regenerateMesh();
};
