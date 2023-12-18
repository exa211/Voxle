#include "ChunkHandler.hpp"

void ChunkHandler::loadChunk(int x, int y, int z) {
  Chunk requestedChunk{x, y, z};

  bool foundRequestedChunk = true;

  // Check if the generated chunks array contains our requested chunk
  // If not set foundRequestedChunk to false
  for(Chunk& chunk : chunksGenerated) {
    if(chunk.getPos() == requestedChunk.getPos()) {
      if(chunk.isGenerated()) {
        foundRequestedChunk = false;
        break;
      }
    }
  }

  // Start generating new chunk
  if(!foundRequestedChunk) {
    LOG(D, "Generating chunk");
  }

}