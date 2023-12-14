#include "Voxelate.h"
#include "Renderer/PrimitiveRenderer.h"
#include "UI/UserInterface.h"
#include "Resource/ResourceHandler.h"

#include "VulkanPipeline/Pipeline/RenderPass.h"
#include "VulkanPipeline/Pipeline/Commandbuffer.h"

#include <FastNoise/FastNoise.h>

#include <VulkanPipeline/Validation/VulkanValidationLayer.h>
#include <VulkanPipeline/VulkanDebug.h>

#include <deque>

const int CHUNK_SIZE{64};

//This is kinda temporary
unsigned int faceIndicesTemp[] = {
  0, 1, 2,
  2, 3, 0
};
// BACK FACE ------
std::vector<uint8_t> backFace = {
  1, 0, 0,
  0, 0, 0,
  0, 1, 0,
  1, 1, 0,
};
// TOP FACE ------
std::vector<uint8_t> topFace = {
  0, 1, 1, // Top front left // 0
  1, 1, 1, // 1
  1, 1, 0, // 2
  0, 1, 0, // 3
};
// BOTTOM FACE ------
std::vector<uint8_t> botFace = {
  0, 0, 0,
  1, 0, 0,
  1, 0, 1,
  0, 0, 1,
};
// LEFT FACE ------
std::vector<uint8_t> leftFace = {
  0, 0, 0,
  0, 0, 1,
  0, 1, 1,
  0, 1, 0,
};
// RIGHT FACE ------
std::vector<uint8_t> rightFace = {
  1, 0, 1,
  1, 0, 0,
  1, 1, 0,
  1, 1, 1,
};

// FRONT FACE ------
std::vector<uint8_t> frontFace = {
  0, 0, 1,// 0 lower left
  1, 0, 1, // 1 lower right
  1, 1, 1, // 2 upper right
  0, 1, 1, // 3 upper left
};

std::vector<glm::vec2> uvs = {
  {0, 0},
  {1, 0},
  {1, 1},
  {0, 1}
};

static void callback_glfwWindowResized(GLFWwindow *window, int w, int h) {
  E_Data::i()->vkInstWrapper.framebufferWasResized = true;
}

static void callback_glfwMousePosition(GLFWwindow *window, double x, double y) {
  E_Data::i()->cursorPos = glm::vec2(x / W_WIDTH, y / W_HEIGHT);
}

static void callback_glfwKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(key == GLFW_KEY_H && action == GLFW_PRESS) {
    LOG(I, "pressed key h");
    //TODO: Implement DebugPipeline
  }
}

void Voxelate::run() {
  LOG(I, "Running engine...");
  initEngine();
}

void Voxelate::initEngine() {
  initWindow();
  initVulkan();

  initScene();

  initGui();
  loop();
  clean();
}

void Voxelate::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // WINDOW DECORATION

  E_Data::i()->window = glfwCreateWindow(W_WIDTH, W_HEIGHT, E_Data::i()->ver.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(E_Data::i()->window, this);

  // Register GLFW Callbacks
  glfwSetFramebufferSizeCallback(E_Data::i()->window, callback_glfwWindowResized);
  glfwSetCursorPosCallback(E_Data::i()->window, callback_glfwMousePosition);
  glfwSetKeyCallback(E_Data::i()->window, callback_glfwKey);

  GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *vidMode = glfwGetVideoMode(primaryMonitor);
  int cX = (vidMode->width / 2) - W_WIDTH / 2;
  int cY = (vidMode->height / 2) - W_HEIGHT / 2;

  glfwSetWindowPos(E_Data::i()->window, cX, cY);

  glfwGetFramebufferSize(E_Data::i()->window, &E_Data::i()->w_frameBuffer, &E_Data::i()->h_frameBuffer);
}

void Voxelate::initVulkan() {
  // Vk Setup Initialization
  VkSetup::createVulkanInstance();
  VkSetup::createDebugMessenger();

  VkSetup::createSurface();

  VkSetup::selectPhysicalDevice();
  VkSetup::createLogicalDevice();

  Pipeline::createCommandPool();

  VkSetup::createSwapchain();
  VkSetup::createImageViews();

  VkSetup::createSampler();

  // TODO: Cache textures in map or something else so we can delete these after and free memory
  VulkanImage::Image leaveTexture{};
  Resources::createTexture(leaveTexture, "../res/texture/dirt.png");

  // Descriptors
  VkDescriptorSetLayout descriptorLayout = VkSetup::createDescriptorSetLayout();
  VkSetup::createDescriptorPool();
  VkSetup::createDescriptorSets();
  VkSetup::populateDescriptors(leaveTexture.image);

  Pipeline::createDepthBufferingObjects();

  // Graphics Pipeline
  RenderPass::create();
  Pipeline::createGraphicsPipeline(descriptorLayout);

  Pipeline::createFramebuffers();

  Commandbuffer::create();

  Pipeline::createSyncObjects();
}

int getBlock(std::deque<int>& blocks, int x, int y, int z) {
  int index = x + (y * CHUNK_SIZE) + (z * CHUNK_SIZE * CHUNK_SIZE);
  if(index < 0 || index > blocks.size()) return 0;
  return blocks[index];
}

int from1D(int x, int y, int z) {
  int index = x + CHUNK_SIZE * (y + CHUNK_SIZE * z);
  if(index < 0) return 0;
  return index;
}

// Initializes the scene
void Voxelate::initScene() {
  Scene mainScene{};

  LOG(I, "FAST NOISE: Supported SIMD Level: " + std::to_string(FastNoise::SUPPORTED_SIMD_LEVELS));

  auto fnSimplex = FastNoise::New<FastNoise::Simplex>();
  auto fnFractal = FastNoise::New<FastNoise::FractalFBm>();

  fnFractal->SetSource(fnSimplex);
  fnFractal->SetOctaveCount(5);

  std::vector<float> noise(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
  fnFractal->GenUniformGrid3D(noise.data(), 0, 0, 0, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.2f, 1337);

  std::vector<Vertex> verticesCombined;
  std::vector<uint32_t> indicesCombined;

  std::deque<int> blocks;

  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        int index = from1D(x, y, z);
        float val = noise[index];

        if(val <= 0.12f) {
          blocks.push_back(0);
          continue;
        }

        blocks.push_back(1);
      }
    }
  }


  int vertIndex{0};
  int indicesCount{0};

  // Some performance measure 128x128x128 chunk uses 10mb RAM and rougly 2Gig VRAM
  // We can reduce the VRAM drastically with packing the verts into a single integer.
  // And then unpacking them in the shader, unpacking is really fast because GPU handles it better.

  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        int block = getBlock(blocks, x, y, z);
        if(block == 0) continue;

        int blockFront = getBlock(blocks, x, y, z + 1);
        int blockBack = getBlock(blocks, x, y, z - 1);
        int blockRight = getBlock(blocks, x + 1, y, z);
        int blockLeft = getBlock(blocks, x - 1, y, z);
        int blockTop = getBlock(blocks, x, y + 1, z);
        int blockBot = getBlock(blocks, x, y - 1, z);

        glm::vec3 pos{x, y, z};

        // Front face
        if(z == CHUNK_SIZE-1 || blockFront == 0) {
          for (int i = 0; i < 4; i++) {
            glm::vec2 uv = uvs[i]; // <- Face UV's

            unsigned int vertX = frontFace[vertIndex++] + pos.x;
            unsigned int vertY = frontFace[vertIndex++] + pos.y;
            unsigned int vertZ = frontFace[vertIndex++] + pos.z;

            verticesCombined.push_back(Vertex{{vertX, vertY, vertZ}, {x*0.2f, 1, 1}, {uv.x, uv.y}});
          }
          for (uint16_t i: faceIndicesTemp) {
            indicesCombined.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Back face
        if(z == 0 || blockBack == 0) {
          for (int i = 0; i < 4; i++) {
            glm::vec2 uv = uvs[i]; // <- Face UV's
            unsigned int vertX = backFace[vertIndex++] + pos.x;
            unsigned int vertY = backFace[vertIndex++] + pos.y;
            unsigned int vertZ = backFace[vertIndex++] + pos.z;
            verticesCombined.push_back(Vertex{{vertX,    vertY, vertZ}, {x * 0.2f, 1, 1}, {uv.x, uv.y}});
          }
          for (uint16_t i: faceIndicesTemp) {
            indicesCombined.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Right face
        if(x == CHUNK_SIZE - 1 || blockRight == 0) {
          for (int i = 0; i < 4; i++) {
            glm::vec2 uv = uvs[i]; // <- Face UV's
            unsigned int vertX = rightFace[vertIndex++] + pos.x;
            unsigned int vertY = rightFace[vertIndex++] + pos.y;
            unsigned int vertZ = rightFace[vertIndex++] + pos.z;
            verticesCombined.push_back(Vertex{{vertX,    vertY, vertZ}, {x * 0.2f, 1, 1}, {uv.x, uv.y}});
          }
          for (uint16_t i: faceIndicesTemp) {
            indicesCombined.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Left face
        if(x == 0 || blockLeft == 0) {
          for (int i = 0; i < 4; i++) {
            glm::vec2 uv = uvs[i]; // <- Face UV's
            unsigned int vertX = leftFace[vertIndex++] + pos.x;
            unsigned int vertY = leftFace[vertIndex++] + pos.y;
            unsigned int vertZ = leftFace[vertIndex++] + pos.z;
            verticesCombined.push_back(Vertex{{vertX,    vertY, vertZ}, {x * 0.2f, 1, 1}, {uv.x, uv.y}});
          }
          for (uint16_t i: faceIndicesTemp) {
            indicesCombined.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Top face
        if(y == CHUNK_SIZE-1 || blockTop == 0) {
          for (int i = 0; i < 4; i++) {
            glm::vec2 uv = uvs[i]; // <- Face UV's
            unsigned int vertX = topFace[vertIndex++] + pos.x;
            unsigned int vertY = topFace[vertIndex++] + pos.y;
            unsigned int vertZ = topFace[vertIndex++] + pos.z;
            verticesCombined.push_back(Vertex{{vertX,    vertY, vertZ}, {x * 0.2f, 1, 1}, {uv.x, uv.y}});
          }
          for (uint16_t i: faceIndicesTemp) {
            indicesCombined.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

        // Bot face
        if(y == 0 || blockBot == 0) {
          for (int i = 0; i < 4; i++) {
            glm::vec2 uv = uvs[i]; // <- Face UV's
            unsigned int vertX = botFace[vertIndex++] + pos.x;
            unsigned int vertY = botFace[vertIndex++] + pos.y;
            unsigned int vertZ = botFace[vertIndex++] + pos.z;
            verticesCombined.push_back(Vertex{{vertX,    vertY, vertZ}, {x * 0.2f, 1, 1}, {uv.x, uv.y}});
          }
          for (uint16_t i: faceIndicesTemp) {
            indicesCombined.push_back(i + indicesCount);
          }
          indicesCount += 4;
          vertIndex = 0;
        }

      }
    }
  }

  std::cout << "Chunk Vert Count: " << verticesCombined.size() << std::endl;
  std::cout << "Chunk Index Count: " << indicesCombined.size() << std::endl;

  Mesh combinedMesh{};
  combinedMesh.vertexBuffer = Buffer::createVertexBuffer(verticesCombined);
  combinedMesh.indexBuffer = Buffer::createIndexBuffer(indicesCombined);
  mainScene.meshesInScene.push_back(combinedMesh);

  LOG(I, "Pushing mesh into scene");

  SceneManager::i()->curScene = mainScene;
  SceneManager::i()->scenesLoaded.push_back(mainScene);
}

void Voxelate::initGui() {
  IMGUI_CHECKVERSION();
  UI::initUserInterface();
}

void Voxelate::update(float deltaTime) {
  cam.update(E_Data::i()->window, deltaTime);
}

void Voxelate::loop() {
  // RENDER PROFILING
  float deltaSeconds = 0.0f;
  double timeStamp = glfwGetTime();

  /*
   *  LOOP
   */

  while (!glfwWindowShouldClose(E_Data::i()->window)) {
    const double newTimeStamp = glfwGetTime();
    deltaSeconds = (float) (newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;
    E_Data::i()->frameProfiler.tick(deltaSeconds);

    //float ms = 1000/time;

    glfwPollEvents();

    /*
   *  UPDATE
   */
    update(deltaSeconds);

    // Future dockspace layout
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    // Main Entry for rendering the Engine User Interface
    UI::renderMainInterface();

    //ImGui::ShowDemoWindow();

    //ImGui::ShowMetricsWindow();
    ImGui::Render();

    // ACTUAL RENDERING
    PrimitiveRenderer::render(cam);
  }

  vkDeviceWaitIdle(E_Data::i()->vkInstWrapper.device);

}

void Voxelate::clean() {
  //TODO: Destroy everything else """ATM""" THIS SHOULD BE OK BECAUSE WINDOWS CLEANS MEMORY AFTER AN EXE WAS CLOSED
  VulkanInstance &vki = E_Data::i()->vkInstWrapper;
  VkDevice &device = vki.device;

  // Cleanup Depth Buffering Resources
  vkDestroyImageView(device, E_Data::i()->vkInstWrapper.depthImage.vkImageView, nullptr);
  vkDestroyImage(device, E_Data::i()->vkInstWrapper.depthImage.vkImage, nullptr);
  vkFreeMemory(device, E_Data::i()->vkInstWrapper.depthImage.vkImageMemory, nullptr);

  VkSetup::cleanupOldSwapchain(device);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(device, vki.uniformBuffers[i], nullptr);
    vkFreeMemory(device, vki.uniformBufferMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(device, vki.descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, vki.descriptorSetLayout, nullptr);

  ImGui_ImplVulkan_Shutdown();

  // Free's the buffer and the deviceMemory for each mesh in the scene
  for (const Scene &s: SceneManager::SceneManager::i()->scenesLoaded) {
    for (const Mesh &m: s.meshesInScene) {
      vkDestroyBuffer(device, m.vertexBuffer.buffer, nullptr);
      vkDestroyBuffer(device, m.indexBuffer.indexBuffer, nullptr);

      vkFreeMemory(device, m.vertexBuffer.bufferMemory, nullptr);
      vkFreeMemory(device, m.indexBuffer.indexBufferMemory, nullptr);
    }
  }

  vkDestroyPipeline(device, vki.pipeline, nullptr);
  vkDestroyPipelineLayout(device, vki.pipelineLayout, nullptr);

  vkDestroyRenderPass(device, vki.renderPass, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vkDestroySemaphore(device, vki.imageAvailableSemas[i], nullptr);
    vkDestroySemaphore(device, vki.renderFinishedSemas[i], nullptr);
    vkDestroyFence(device, vki.inFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(device, vki.commandPool, nullptr);

  vkDestroyDevice(device, nullptr);

  if(enableValidation) {
    VulkanDebug::destroyDebugUtilsMessengerEXT(vki.vkInstance, vki.debugMessenger, nullptr);
  }

  vkDestroyInstance(vki.vkInstance, nullptr);

  LOG(I, "Cleaned everything up");
}