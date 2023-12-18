#pragma once

#include <vector>

#include "Chunk.hpp"

class ChunkHandler {
public:
  void loadChunk(int x, int y, int z);
  Chunk& getChunk(int x, int y, int z);
private:
  std::vector<Chunk> chunksGenerated;
  std::vector<Chunk> chunksLoaded;
};
