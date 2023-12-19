#pragma once

#include "VulkanPipeline/Pipeline/Buffer/Buffer.h"
#include <deque>

#include "Block.hpp"
#include "FastNoise/SmartNode.h"
#include "Renderer/Mesh/Mesh.h"

#define CHUNK_SIZE 32

struct ChunkMesh {
  std::vector<BlockVertex> vertices{};
  std::vector<uint32_t> indices{};
  Mesh mesh{};
};

class Chunk {
public:
  Chunk(int xPos, int yPos, int zPos) {
    pos = {xPos, yPos, zPos};
  }

  [[nodiscard]] bool isChunkEmpty() const;
  [[nodiscard]] bool isLoaded() const;
  [[nodiscard]] bool isGenerated() const;

  void setChunkLoaded(bool loaded);
  void setChunkGenerated(bool generated);

  [[nodiscard]] glm::ivec3* getPos();

  Material getBlockUnsafe(int x, int y, int z);
  Material& getBlock(int xSafe, int ySafe, int zSafe);

  std::deque<Material>& getBlocks();

  ChunkMesh& getChunkMesh();

  void generate(std::vector<float>& noise);
  void regenerateMesh();

private:
  glm::ivec3 pos{}; // Chunk pos normalized

  std::deque<Material> blocks{};
  ChunkMesh chunkMesh{};
  bool bGenerated = false;
  bool bEmpty = true;
  bool bLoaded = false;
};
