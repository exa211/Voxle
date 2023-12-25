#include "Voxelate.h"
#include "Renderer/PrimitiveRenderer.h"
#include "UI/UserInterface.h"
#include "Resource/ResourceHandler.h"

#include "VulkanPipeline/Pipeline/RenderPass.h"
#include "VulkanPipeline/Pipeline/Commandbuffer.h"

#include <FastNoise/FastNoise.h>

#include <VulkanPipeline/Validation/VulkanValidationLayer.h>
#include <VulkanPipeline/VulkanDebug.h>

#include <World/Chunk.hpp>

#include <Scene/SceneManager.h>

static void callback_glfwWindowResized(GLFWwindow *window, int w, int h) {
  EngineData::i()->w_frameBuffer = w;
  EngineData::i()->h_frameBuffer = h;
  EngineData::i()->vkInstWrapper.framebufferWasResized = true;
}

static void callback_glfwMousePosition(GLFWwindow *window, double x, double y) {
  EngineData::i()->cursorPos = glm::vec2(x / W_WIDTH, y / W_HEIGHT);
}

static void callback_glfwKey(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_H && action == GLFW_PRESS) {
    LOG(D, "Toggled Wireframe Visualization");

    int &currentShader = EngineData::i()->vkInstWrapper.currentShader;

    if (currentShader == 0) {
      currentShader = 1;
    } else {
      currentShader = 0;
    }
  }

  if (key == GLFW_KEY_B && action == GLFW_PRESS) {
    LOG(D, "Toggled Bounding Box Visualization");

    for (Chunk *chunk: EngineData::i()->chunkHandler.getChunksGenerated()) {
      auto boundingBoxDefinition = chunk->getChunkMesh().boundingBox.getMeshDefinition();

      Mesh boundingBoxMesh{};
      boundingBoxMesh.shader = &EngineData::i()->vkInstWrapper.pipelines.find("chunkWireframe")->second;
      boundingBoxMesh.vertexBuffer = Buffers::createVertexBuffer(boundingBoxDefinition.first);
      boundingBoxMesh.indexBuffer = Buffers::createIndexBuffer(boundingBoxDefinition.second);
      boundingBoxMesh.meshRenderData.transformMatrix = glm::translate(glm::mat4(1),
                                                                      {chunk->getPos().x, chunk->getPos().y,
                                                                       chunk->getPos().z});

      int index{};

      for (Vertex vert: boundingBoxDefinition.first) {
        LOG(D, std::to_string(index) + " | " + Util::stringFromVec3(vert.pos));
        index++;
      }

      index = 0;

      SceneManager::i()->curScene.meshesInScene.push_back(boundingBoxMesh);
    }

  }

}

void Voxelate::run() {
  initWindow();

  initVulkan();

  ThreadSet threadSet{1, 1, 1};
  EngineData::i()->threadPool.start(threadSet, 8);

  initScene();

  initGui();
  loop();
  clean();
}

/**
 *  Create the Applications window.
 **/
void Voxelate::initWindow() {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // We want no opengl api because we are using Vulkan
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE); // WINDOW DECORATION

  EngineData::i()->window = glfwCreateWindow(W_WIDTH, W_HEIGHT, EngineData::i()->ver.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(EngineData::i()->window, this);

  // Register GLFW Callbacks
  glfwSetFramebufferSizeCallback(EngineData::i()->window, callback_glfwWindowResized);
  glfwSetCursorPosCallback(EngineData::i()->window, callback_glfwMousePosition);
  glfwSetKeyCallback(EngineData::i()->window, callback_glfwKey);

  GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *vidMode = glfwGetVideoMode(primaryMonitor);
  int cX = (vidMode->width / 2) - static_cast<int>(W_WIDTH / 2);
  int cY = (vidMode->height / 2) - static_cast<int>(W_HEIGHT / 2);

  glfwSetWindowPos(EngineData::i()->window, cX, cY);

  glfwGetFramebufferSize(EngineData::i()->window, &EngineData::i()->w_frameBuffer, &EngineData::i()->h_frameBuffer);
}

/**
 *  Initialize all Vulkan related rendering stuff.
 **/
void Voxelate::initVulkan() {
  // Vk Setup Initialization
  VkSetup::createVulkanInstance();
  VkSetup::createDebugMessenger();

  VkSetup::createSurface();

  // Devices
  VkSetup::selectPhysicalDevice();
  VkSetup::createLogicalDevice();

  // Vma Allocator creation
  VkSetup::createVmaAllocator();

  VulkanPipeline::createCommandPool();

  VkSetup::createSwapchain();
  VkSetup::createImageViews();

  VkSetup::createSampler();

  // TODO: Cache textures in map or something else so we can delete these after and free memory
  VulkanImage::Image texture{};
  Resources::createTexture(texture, "cobblestone.png");

  // Descriptors
  VkDescriptorSetLayout descriptorLayout = VkSetup::createDescriptorSetLayout();
  VkSetup::createDescriptorPool();
  VkSetup::createDescriptorSets();
  VkSetup::populateDescriptors(texture.image);

  VulkanPipeline::createDepthBufferingObjects();

  // Renderpass creation
  // TODO: Abstraction
  RenderPass::create();

  // Global Shader Pipeline / UBER Shader
  VulkanPipeline::Pipeline globalPipeline{};
  globalPipeline.pipelineName = "global";
  globalPipeline.bindShader("shader");
  globalPipeline.setVertexDescriptions(BlockVertex::getBindingDescription(), BlockVertex::getAttributeDescriptions());
  globalPipeline.setDescriptorLayout(descriptorLayout);
  globalPipeline.setRenderPass(EngineData::i()->vkInstWrapper.renderPass);
  globalPipeline.setPolygonMode(VK_POLYGON_MODE_FILL);
  globalPipeline.build();

  // Debug Pipeline
  VulkanPipeline::Pipeline debugPipeline{};
  debugPipeline.pipelineName = "debug";
  debugPipeline.bindShader("wireframe");
  debugPipeline.setVertexDescriptions(BlockVertex::getBindingDescription(), BlockVertex::getAttributeDescriptions());
  debugPipeline.setDescriptorLayout(descriptorLayout);
  debugPipeline.setRenderPass(EngineData::i()->vkInstWrapper.renderPass);
  debugPipeline.setPolygonMode(VK_POLYGON_MODE_LINE);
  debugPipeline.build();

  // Debug Pipeline
  VulkanPipeline::Pipeline chunkWireframe{};
  chunkWireframe.pipelineName = "chunkWireframe";
  chunkWireframe.bindShader("debug");
  chunkWireframe.setVertexDescriptions(Vertex::getBindingDescription(), Vertex::getAttributeDescriptions());
  chunkWireframe.setDescriptorLayout(descriptorLayout);
  chunkWireframe.setRenderPass(EngineData::i()->vkInstWrapper.renderPass);
  chunkWireframe.setPolygonMode(VK_POLYGON_MODE_FILL);
  chunkWireframe.setCulling(VK_CULL_MODE_NONE);
  chunkWireframe.build();

  VulkanPipeline::createFramebuffers();

  Commandbuffer::create();

  VulkanPipeline::createSyncObjects();
}

// Initializes the scene
void Voxelate::initScene() {
  Scene mainScene{};

  LOG(I, "FAST NOISE: Supported SIMD Level: " + std::to_string(FastNoise::SUPPORTED_SIMD_LEVELS));

  LOG(D, "Settings scenes");

  SceneManager::i()->curScene = mainScene;
  SceneManager::i()->scenesLoaded.push_back(mainScene);

}

void Voxelate::initGui() {
  IMGUI_CHECKVERSION();
  UI::initUserInterface();
}

const int renderDistance = 32;
const int renderDistanceY = 2;

float inline distanceSq(glm::vec3 &a, glm::vec3 &b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  float dz = a.z - b.z;
  return (dx * dx) + (dy * dy) + (dz * dz);
}

void Voxelate::update(float deltaTime) {
  cam.update(EngineData::i()->window, deltaTime);

  // TODO: We need to multithread this
  ChunkHandler &ch = EngineData::i()->chunkHandler;
  int x = cam.position.x / CHUNK_SIZE;
  int y = cam.position.y / CHUNK_SIZE;
  int z = cam.position.z / CHUNK_SIZE;
  glm::vec3 camPos{x,y,z};
  for (int xc = -renderDistance + x; xc < renderDistance + x; xc++) {
    for (int yc = -renderDistanceY + y; yc < renderDistanceY + y; yc++) {
      for (int zc = -renderDistance + z; zc < renderDistance + z; zc++) {
        glm::vec3 pos{xc, yc, zc};

        float distToChunkFromPlayer = distanceSq(pos, camPos);

        if (distToChunkFromPlayer <= renderDistance + 2) {
          // Check for existence or currently in queue
          if (ch.getChunk(pos) != nullptr || ch.isChunkInQueue(pos)) continue;

          ch.addChunkToQueue({xc, yc, zc});
        }
      }
    }
  }
//
  for (Chunk *chunk: ch.getChunksGenerated()) {
    if (chunk->isLoaded()) continue;
    if (chunk->isChunkEmpty() || !chunk->isMeshed()) continue;

    chunk->setChunkLoaded(true);

    // Face Construction done -> create buffers in mesh struct

    Mesh &mesh = chunk->getChunkMesh().mesh;
    mesh.vertexBuffer = Buffers::createBlockVertexBuffer(chunk->getChunkMesh().vertices);
    mesh.indexBuffer = Buffers::createIndexBuffer(chunk->getChunkMesh().indices);
    mesh.meshRenderData.transformMatrix = glm::translate(glm::mat4(1), {chunk->getPos().x * CHUNK_SIZE,
                                                                        chunk->getPos().y * CHUNK_SIZE,
                                                                        chunk->getPos().z * CHUNK_SIZE});

    SceneManager::i()->curScene.meshesInScene.push_back(chunk->getChunkMesh().mesh);
  }
}

void Voxelate::loop() {
  // RENDER PROFILING
  float deltaSeconds = 0.0f;
  double timeStamp = glfwGetTime();

  while (!glfwWindowShouldClose(EngineData::i()->window)) {
    const double newTimeStamp = glfwGetTime();
    deltaSeconds = (float) (newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;
    EngineData::i()->frameProfiler.tick(deltaSeconds);

    glfwPollEvents();

    this->update(deltaSeconds);

    // Main Entry for rendering the Engine User Interface
    UI::renderMainInterface(cam);

    // >- RENDERING -<
    PrimitiveRenderer::render(cam);
  }

  vkDeviceWaitIdle(EngineData::i()->vkInstWrapper.device);
}

void Voxelate::clean() {
  //TODO: Destroy everything else """ATM""" THIS SHOULD BE OK BECAUSE WINDOWS CLEANS MEMORY AFTER AN EXE WAS CLOSED
  VulkanInstance &vki = EngineData::i()->vkInstWrapper;
  VkDevice &device = vki.device;

  VmaAllocator &allocator = vki.vmaAllocator;

  // Cleanup Depth Buffering Resources
  vkDestroyImageView(device, EngineData::i()->vkInstWrapper.depthImage.vkImageView, nullptr);
  vkDestroyImage(device, EngineData::i()->vkInstWrapper.depthImage.vkImage, nullptr);
  vkFreeMemory(device, EngineData::i()->vkInstWrapper.depthImage.vkImageMemory, nullptr);

  VkSetup::cleanupOldSwapchain(device);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vmaDestroyBuffer(allocator, vki.uniformBuffers[i].buffer, nullptr);
  }

  vkDestroyDescriptorPool(device, vki.descriptorPool, nullptr);
  vkDestroyDescriptorSetLayout(device, vki.descriptorSetLayout, nullptr);

  ImGui_ImplVulkan_Shutdown();

  // Free's the buffer and the deviceMemory for each mesh in the scene
  for (const Scene &s: SceneManager::SceneManager::i()->scenesLoaded) {
    for (const Mesh &m: s.meshesInScene) {
      vkDestroyBuffer(device, m.vertexBuffer.buffer, nullptr);
      vkDestroyBuffer(device, m.indexBuffer.indexBuffer, nullptr);

      vmaFreeMemory(allocator, m.vertexBuffer.allocation);
      vmaFreeMemory(allocator, m.indexBuffer.allocation);
    }
  }

  vkDestroyPipeline(device, vki.globalPipeline.getPipeline(), nullptr);
  vkDestroyPipelineLayout(device, vki.globalPipeline.getPipelineLayout(), nullptr);

  vkDestroyRenderPass(device, vki.renderPass, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    vkDestroySemaphore(device, vki.imageAvailableSemas[i], nullptr);
    vkDestroySemaphore(device, vki.renderFinishedSemas[i], nullptr);
    vkDestroyFence(device, vki.inFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(device, vki.commandPool, nullptr);

  vkDestroyDevice(device, nullptr);

  if (enableValidation) {
    VulkanDebug::destroyDebugUtilsMessengerEXT(vki.vkInstance, vki.debugMessenger, nullptr);
  }

  vkDestroyInstance(vki.vkInstance, nullptr);

  LOG(I, "Cleaned everything up");
}