#include "Chunk.hpp"

#include <functional>

#include <glm/gtc/packing.hpp>

#include <Engine.h>

inline const unsigned int faceIndices[] = {
  0, 1, 2,
  2, 3, 0
};

// FRONT FACE ------
inline const std::vector<signed char> frontFace = {
  0, 0, 1,// 0 lower left
  1, 0, 1, // 1 lower right
  1, 1, 1, // 2 upper right
  0, 1, 1, // 3 upper left
};
// BACK FACE ------
inline const std::vector<signed char> backFace = {
  1, 0, 0,
  0, 0, 0,
  0, 1, 0,
  1, 1, 0,
};
// TOP FACE ------
inline const std::vector<signed char> topFace = {
  0, 1, 1, // Top front left // 0
  1, 1, 1, // 1
  1, 1, 0, // 2
  0, 1, 0, // 3
};
// BOTTOM FACE ------
inline const std::vector<signed char> botFace = {
  0, 0, 0,
  1, 0, 0,
  1, 0, 1,
  0, 0, 1,
};
// LEFT FACE ------
inline const std::vector<signed char> leftFace = {
  0, 0, 0,
  0, 0, 1,
  0, 1, 1,
  0, 1, 0,
};
// RIGHT FACE ------
inline const std::vector<signed char> rightFace = {
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  1, 1, 1,
};

glm::ivec3 *Chunk::getPos() {
  return &pos;
}

Material *Chunk::getBlockUnsafe(int x, int y, int z) {
  int index = x + (y * CHUNK_SIZE) + (z * CHUNK_SIZE * CHUNK_SIZE);
  if (index < 0 || index > CHUNK_VOLUME) return nullptr;
  return &blocks[index];
}

Material &Chunk::getBlock(int xSafe, int ySafe, int zSafe) {
  int index = xSafe + (ySafe * CHUNK_SIZE) + (zSafe * CHUNK_SIZE * CHUNK_SIZE);
  return blocks[index];
}

ChunkMesh &Chunk::getChunkMesh() {
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

// >---- GENERATION -----<

inline const bool solid(Material mat) {
  return mat.id > 0;
}

bool Chunk::generate(std::vector<float> &noise) {

  // If we didn't find a single solid block just abort generating this chunk.
  auto hasSolid = [](float val) { return val < 0.5f; };
  if(std::find_if(noise.begin(), noise.end(), hasSolid) == noise.end()) return false;

  int index{};

  // Iterate over noise and set blocks
  for (int z = 0; z < CHUNK_SIZE; z++) {
    for (int y = 0; y < CHUNK_SIZE; y++) {
      for (int x = 0; x < CHUNK_SIZE; x++) {
        float nVal = noise[index++];
        if (nVal >= 0.5f) {
          blocks.push_back(Materials::AIR);
          continue;
        }
        bEmpty = false;
        blocks.push_back(Materials::SOLID);
      }
    }
  }

  regenerateMesh();

  return true;
}

void Chunk::regenerateMesh() {
  if (bEmpty) return;

  int vertIndex{0};
  int indicesCount{0};

  chunkMesh.vertices.reserve(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
  chunkMesh.indices.reserve(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

  static int texture = 0;
  static int lightLevel = 1;

  // We could reverse this, so we iterate only through empty blocks and set faces for solid blocks instead

  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        Material *block = getBlockUnsafe(x, y, z);
        if (!solid(*block)) continue;

        Material *blockFront = getBlockUnsafe(x, y, z + 1);
        Material *blockBack = getBlockUnsafe(x, y, z - 1);
        Material *blockRight = getBlockUnsafe(x + 1, y, z);
        Material *blockLeft = getBlockUnsafe(x - 1, y, z);
        Material *blockTop = getBlockUnsafe(x, y + 1, z);
        Material *blockBot = getBlockUnsafe(x, y - 1, z);

        glm::ivec3 vpos{x, y, z};

        // Front face
        if (z == CHUNK_SIZE - 1 || !solid(*blockFront)) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = frontFace[vertIndex++] + vpos.x;
            unsigned int vertY = frontFace[vertIndex++] + vpos.y;
            unsigned int vertZ = frontFace[vertIndex++] + vpos.z;
            unsigned int vert = vertX | vertY << 6 | vertZ << 12 | lightLevel << 18 | i << 21 | texture << 23;

            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Back face
        if (z == 0 || !solid(*blockBack)) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = backFace[vertIndex++] + vpos.x;
            unsigned int vertY = backFace[vertIndex++] + vpos.y;
            unsigned int vertZ = backFace[vertIndex++] + vpos.z;
            unsigned int vert = vertX | vertY << 6 | vertZ << 12 | lightLevel << 18 | i << 21 | texture << 23;
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Right face
        if (x == CHUNK_SIZE - 1 || !solid(*blockRight)) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = rightFace[vertIndex++] + vpos.x;
            unsigned int vertY = rightFace[vertIndex++] + vpos.y;
            unsigned int vertZ = rightFace[vertIndex++] + vpos.z;
            unsigned int vert = vertX | vertY << 6 | vertZ << 12 | lightLevel << 18 | i << 21 | texture << 23;
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Left face
        if (x == 0 || !solid(*blockLeft)) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = leftFace[vertIndex++] + vpos.x;
            unsigned int vertY = leftFace[vertIndex++] + vpos.y;
            unsigned int vertZ = leftFace[vertIndex++] + vpos.z;

            unsigned int vert = vertX | vertY << 6 | vertZ << 12 | lightLevel << 18 | i << 21 | texture << 23;
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Top face
        if (y == CHUNK_SIZE - 1 || !solid(*blockTop)) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = topFace[vertIndex++] + vpos.x;
            unsigned int vertY = topFace[vertIndex++] + vpos.y;
            unsigned int vertZ = topFace[vertIndex++] + vpos.z;
            unsigned int vert = vertX | vertY << 6 | vertZ << 12 | lightLevel << 18 | i << 21 | texture << 23;
            chunkMesh.vertices.emplace_back(BlockVertex{vert});
          }
          for (uint8_t i: faceIndices) {
            chunkMesh.indices.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Bot face
        if (y == 0 || !solid(*blockBot)) {
          for (int i = 0; i < 4; i++) {
            unsigned int vertX = botFace[vertIndex++] + vpos.x;
            unsigned int vertY = botFace[vertIndex++] + vpos.y;
            unsigned int vertZ = botFace[vertIndex++] + vpos.z;
            unsigned int vert = vertX | vertY << 6 | vertZ << 12 | lightLevel << 18 | i << 21 | texture << 23;
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

  if (indicesCount == 0) bEmpty = true;
}

std::deque<Material> &Chunk::getBlocks() {
  return this->blocks;
}
