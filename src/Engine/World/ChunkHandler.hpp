#pragma once

#include <vector>
#include <queue>
#include <thread>

#include "Chunk.hpp"

class ChunkHandler {
public:
  void generateChunk(const glm::ivec3& pos);
  void createNoiseChunk(const glm::ivec3 &pos);

  Chunk* getChunk(const glm::ivec3& pos);

  void addChunkToQueue(const glm::ivec3& pos);
  bool isChunkInQueue(const glm::ivec3& pos);

  std::vector<Chunk*>& getChunksGenerated();

  std::deque<glm::ivec3>* getChunkGenQueue();

private:

  // TODO: V2 ChunkHandling
  std::deque<glm::ivec3> chunkGenList;
  std::deque<Chunk*> chunkMeshList;
  std::deque<Chunk*> chunkUpdateList;
  std::deque<Chunk*> chunkUnloadList;


  std::vector<glm::ivec3> chunksGenerating;

  std::vector<Chunk*> chunksGenerated;
  std::vector<Chunk*> chunksLoaded;
};
