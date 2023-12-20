#pragma once

#include <vector>

#include "Chunk.hpp"

class ChunkHandler {
public:
  void generateChunk(int x, int y, int z);
  Chunk* getChunk(int x, int y, int z);

  std::vector<glm::ivec3>& getChunksGenerating();

  std::vector<Chunk*> getChunksGenerated();
  std::vector<Chunk*> getChunksLoaded();
private:
  std::vector<glm::ivec3> chunksGenerating;

  std::vector<Chunk*> chunksGenerated;
  std::vector<Chunk*> chunksLoaded;
};
