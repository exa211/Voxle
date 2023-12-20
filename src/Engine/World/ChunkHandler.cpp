#include "ChunkHandler.hpp"
#include "Engine.h"

#include <FastNoise/FastNoise.h>

const FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree(
  "DQADAAAApHB9QBAAFK5nQBkAEwApXA8+DQAEAAAAzcwsQBkACQABAAAAAIA/AOxRuD0Ahes5QQEEAAAAAABSuM5AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAI/Cdb0Aj8J1PgB7FC4/");

void ChunkHandler::generateChunk(int x, int y, int z) {
  glm::ivec3 chunkPos{x, y, z};

  // Allocate the new chunk on the heap and push back ptr into generatedChunks
  auto pChunk = new Chunk{x, y, z};

  // Add job to thread pool so a thread can pick it up
  EngineData::i()->threadPool.addJob([pChunk, this] {

    std::vector<float> noise(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    fnGenerator->GenUniformGrid3D(noise.data(), pChunk->getPos()->z * CHUNK_SIZE, pChunk->getPos()->y * CHUNK_SIZE,
                                  pChunk->getPos()->x * CHUNK_SIZE, CHUNK_SIZE,
                                  CHUNK_SIZE, CHUNK_SIZE, 0.03f, 69);

    // This is really weird IDK...
    // Basically this checks if the chunk would generate in pre-pass
    // But if it doesn't pass we need to set it anyway but in generate it needs to return to abort the thread and meshing??
    if(pChunk->generate(noise)) {
      pChunk->setChunkGenerated(true);
      chunksGenerated.push_back(pChunk); // <- TODO: This could crash sometime in the future?
      return;
    }
    pChunk->setChunkGenerated(true);
    chunksGenerated.push_back(pChunk); // <- TODO: This could crash sometime in the future?
  });

}

Chunk *ChunkHandler::getChunk(int x, int y, int z) {
  Chunk requestedChunk{x, y, z};

  for (Chunk *chunk: chunksGenerated) {
    if (chunk->getPos() == requestedChunk.getPos()) {
      return chunk;
    }
  }

  return nullptr;
}

std::vector<Chunk *> ChunkHandler::getChunksGenerated() {
  return this->chunksGenerated;
}

std::vector<Chunk *> ChunkHandler::getChunksLoaded() {
  return this->chunksLoaded;
}

std::vector<glm::ivec3> &ChunkHandler::getChunksGenerating() {
  return this->chunksGenerating;
}
