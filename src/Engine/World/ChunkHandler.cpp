#include "ChunkHandler.hpp"
#include "Engine.h"

#include <FastNoise/FastNoise.h>

const FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree(
  "EADNzCxAGQATAClcDz4NAAQAAADNzCxAGQAJAAEAAAAAgD8A7FG4PQCF6zlBAQQAAAAAAFK4zkAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACtejPQ==");

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

    pChunk->generate(noise);
    pChunk->setChunkGenerated(true);
    chunksGenerated.push_back(pChunk); // <- TODO: This could crash sometime in the future?
    LOG(W, std::this_thread::get_id());
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
  return this->chunkGenerating;
}
