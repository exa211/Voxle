#include "Chunk.hpp"

#include <glm/gtc/packing.hpp>

unsigned int faceIndices[] = {
  0, 1, 2,
  2, 3, 0
};

// FRONT FACE ------
std::vector<signed char> frontFace = {
  0, 0, 1,// 0 lower left
  1, 0, 1, // 1 lower right
  1, 1, 1, // 2 upper right
  0, 1, 1, // 3 upper left
};
// BACK FACE ------
std::vector<signed char> backFace = {
  1, 0, 0,
  0, 0, 0,
  0, 1, 0,
  1, 1, 0,
};
// TOP FACE ------
std::vector<signed char> topFace = {
  0, 1, 1, // Top front left // 0
  1, 1, 1, // 1
  1, 1, 0, // 2
  0, 1, 0, // 3
};
// BOTTOM FACE ------
std::vector<signed char> botFace = {
  0, 0, 0,
  1, 0, 0,
  1, 0, 1,
  0, 0, 1,
};
// LEFT FACE ------
std::vector<signed char> leftFace = {
  0, 0, 0,
  0, 0, 1,
  0, 1, 1,
  0, 1, 0,
};
// RIGHT FACE ------
std::vector<signed char> rightFace = {
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  1, 1, 1,
};

glm::ivec3* Chunk::getPos() {
  return &pos;
}

Material Chunk::getBlockUnsafe(int x, int y, int z) {
  int index = x + (y * CHUNK_SIZE) + (z * CHUNK_SIZE * CHUNK_SIZE);
  if (index < 0 || index > blocks.size()) return Materials::AIR;
  return blocks[index];
}

Material& Chunk::getBlock(int xSafe, int ySafe, int zSafe) {
  int index = xSafe + (ySafe * CHUNK_SIZE) + (zSafe * CHUNK_SIZE * CHUNK_SIZE);
  return blocks[index];
}

ChunkMesh& Chunk::getChunkMesh() {
  return chunkMesh;
}

bool Chunk::isChunkEmpty() const {
  return bEmpty;
}

bool Chunk::isGenerated() const {
  return bGenerated;
}

bool Chunk::isLoaded() const {
  return bLoaded;
}

void Chunk::setChunkLoaded(bool loaded) {
  bLoaded = loaded;
}

void Chunk::setChunkGenerated(bool generated) {
  bGenerated = generated;
}

void Chunk::generate(std::vector<float>& noise) {
  int index{0};

  for (int z = 0; z < CHUNK_SIZE; ++z) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int x = 0; x < CHUNK_SIZE; ++x) {
        float val = noise[index++];

        if (val >= 0.92f) {
          blocks.push_back(Materials::AIR);
          continue;
        }

        if(bEmpty) bEmpty = false; // <- If we found a single solid block set the chunk to be not empty
        blocks.push_back(Materials::SOLID);
      }
    }
  }
}

void Chunk::regenerateMesh() {
  if(bEmpty) return;

  int vertIndex{0};
  int indicesCount{0};

  chunkMesh.vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
  chunkMesh.indices.reserve(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        Material block = getBlockUnsafe(x, y, z);
        if (block.id == 0) continue;

        Material blockFront = getBlockUnsafe(x, y, z + 1);
        Material blockBack = getBlockUnsafe(x, y, z - 1);
        Material blockRight = getBlockUnsafe(x + 1, y, z);
        Material blockLeft = getBlockUnsafe(x - 1, y, z);
        Material blockTop = getBlockUnsafe(x, y + 1, z);
        Material blockBot = getBlockUnsafe(x, y - 1, z);

        glm::ivec3 pos{x, y, z};

        // Front face
        if (z == CHUNK_SIZE - 1 || blockFront.id == 0) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = frontFace[vertIndex++] + pos.x;
            unsigned int vertY = frontFace[vertIndex++] + pos.y;
            unsigned int vertZ = frontFace[vertIndex++] + pos.z;
            unsigned int vert = glm::packInt4x8({vertX, vertY, vertZ, i});
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Back face
        if (z == 0 || blockBack.id == 0) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = backFace[vertIndex++] + pos.x;
            unsigned int vertY = backFace[vertIndex++] + pos.y;
            unsigned int vertZ = backFace[vertIndex++] + pos.z;
            unsigned int vert = glm::packInt4x8({vertX, vertY, vertZ, i});
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Right face
        if (x == CHUNK_SIZE - 1 || blockRight.id == 0) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = rightFace[vertIndex++] + pos.x;
            unsigned int vertY = rightFace[vertIndex++] + pos.y;
            unsigned int vertZ = rightFace[vertIndex++] + pos.z;
            unsigned int vert = glm::packInt4x8({vertX, vertY, vertZ, i});
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Left face
        if (x == 0 || blockLeft.id == 0) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = leftFace[vertIndex++] + pos.x;
            unsigned int vertY = leftFace[vertIndex++] + pos.y;
            unsigned int vertZ = leftFace[vertIndex++] + pos.z;
            unsigned int vert = glm::packInt4x8({vertX, vertY, vertZ, i});
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Top face
        if (y == CHUNK_SIZE - 1 || blockTop.id == 0) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = topFace[vertIndex++] + pos.x;
            unsigned int vertY = topFace[vertIndex++] + pos.y;
            unsigned int vertZ = topFace[vertIndex++] + pos.z;
            unsigned int vert = glm::packInt4x8({vertX, vertY, vertZ, i});
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Bot face
        if (y == 0 || blockBot.id == 0) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = botFace[vertIndex++] + pos.x;
            unsigned int vertY = botFace[vertIndex++] + pos.y;
            unsigned int vertZ = botFace[vertIndex++] + pos.z;
            unsigned int vert = glm::packInt4x8({vertX, vertY, vertZ, i});
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

      }
    }
  }

  // Face Construction done -> create buffers in mesh struct

  Mesh& mesh = chunkMesh.mesh;
  mesh.vertexBuffer = Buffers::createBlockVertexBuffer(chunkMesh.vertices);
  mesh.indexBuffer = Buffers::createIndexBuffer(chunkMesh.indices);
  mesh.meshRenderData.transformMatrix = glm::translate(glm::mat4(1), {pos.x * CHUNK_SIZE, pos.y * CHUNK_SIZE, pos.z * CHUNK_SIZE});
}