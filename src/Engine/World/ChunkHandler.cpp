#include "ChunkHandler.hpp"
#include "Engine.h"
#include "Util/Util.hpp"

#include <FastNoise/FastNoise.h>

const FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree(
  "IgAAAIA/CtejPBkAIQAEAAAAAACamRlAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACQAACtcjPQEZAAQAAAAAAGZm5j8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABDQACAAAAAAAAQAcAAGZmZr8AAACAQA==");


bool ChunkHandler::isChunkInQueue(const glm::ivec3& pos) {
  return std::find(chunkGenList.begin(), chunkGenList.end(), pos) != chunkGenList.end();
}

/**
 *  @brief Adds a chunk to the chunkGenList and signals a thread for generating.
 **/
void ChunkHandler::addChunkToQueue(const glm::ivec3 &pos) {

  chunkGenList.push_front(pos);
  EngineData::i()->threadPool.signalBuilderThreads();
}

/**
 *  @brief IMPORTANT: This should never be called. Use addChunkToQueue instead, otherwise it will crash the engine.
 *  This will generate noise for surrounding chunks first and then completely generate the main chunk.
 */
void ChunkHandler::generateChunk(const glm::ivec3 &pos) {
  Chunk* chunk = this->getChunk(pos);

  // If this chunk is not existing
  if(chunk == nullptr) {

    chunk = new Chunk{pos};

    // Step 2: Check for neighbours and generate the missing ones
    glm::ivec3 posTop = {pos.x, pos.y + 1, pos.z};
    glm::ivec3 posBot = {pos.x, pos.y - 1, pos.z};
    glm::ivec3 posLeft = {pos.x - 1, pos.y, pos.z};
    glm::ivec3 posRight = {pos.x + 1, pos.y, pos.z};
    glm::ivec3 posFront = {pos.x, pos.y, pos.z - 1};
    glm::ivec3 posBack = {pos.x, pos.y, pos.z + 1};

    std::vector<float> noise(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
    fnGenerator->GenUniformGrid3D(noise.data(),
                                  chunk->getPos().z * CHUNK_SIZE,
                                  chunk->getPos().y * CHUNK_SIZE,
                                  chunk->getPos().x * CHUNK_SIZE,
                                  CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.03f, 69);

    chunk->generate(noise);

    chunksGenerated.push_back(chunk);
    chunk->regenerateMesh();


    // Check and add to queue for generation
//    if(!getChunk(posTop)) addChunkToQueue(posTop);
//    if(!getChunk(posBot)) addChunkToQueue(posBot);
//    if(!getChunk(posLeft)) addChunkToQueue(posLeft);
//    if(!getChunk(posRight)) addChunkToQueue(posRight);
//    if(!getChunk(posFront)) addChunkToQueue(posFront);
//    if(!getChunk(posBack)) addChunkToQueue(posBack);

  } else {
    // Chunk is already generated we can look up neighbours and maybe mesh it

  }
}

/**
 * @brief Creates a noise chunk for only noise data.
 * This is meant to be executed on the same thread where the main chunk gets generated.
 * Needed so the main chunk can get surrounding noise data for meshing.
 **/
void ChunkHandler::createNoiseChunk(const glm::ivec3 &pos) {

}

Chunk *ChunkHandler::getChunk(const glm::ivec3 &pos) {
  const auto isSamePos = [pos](Chunk *chunk) { return pos == chunk->getPos(); };
  auto it = std::find_if(chunksGenerated.begin(), chunksGenerated.end(), isSamePos);
  if (it == chunksGenerated.end()) return nullptr;
  return *it;
}

std::vector<Chunk *> &ChunkHandler::getChunksGenerated() {
  return this->chunksGenerated;
}

std::deque<glm::ivec3> *ChunkHandler::getChunkGenQueue() {
  return &this->chunkGenList;
}
