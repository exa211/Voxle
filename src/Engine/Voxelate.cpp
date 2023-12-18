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

#include <World/Chunk.hpp>

#include <Scene/SceneManager.h>

static void callback_glfwWindowResized(GLFWwindow *window, int w, int h) {
  EngineData::i()->vkInstWrapper.framebufferWasResized = true;
}

static void callback_glfwMousePosition(GLFWwindow *window, double x, double y) {
  EngineData::i()->cursorPos = glm::vec2(x / W_WIDTH, y / W_HEIGHT);
}

static void callback_glfwKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if(key == GLFW_KEY_H && action == GLFW_PRESS) {
    LOG(I, "pressed key h");
    //TODO: Implement DebugPipeline
  }
}

void Voxelate::run() {
  initWindow();

  EngineData::i()->threadPool.start(2);

  initVulkan();

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
  Resources::createTexture(texture, "stone.png");

  // Descriptors
  VkDescriptorSetLayout descriptorLayout = VkSetup::createDescriptorSetLayout();
  VkSetup::createDescriptorPool();
  VkSetup::createDescriptorSets();
  VkSetup::populateDescriptors(texture.image);

  VulkanPipeline::createDepthBufferingObjects();

  // Renderpass creation
  // TODO: Abstraction
  RenderPass::create();

  // Test OOP Pipeline
  VulkanPipeline::Pipeline debugPipeline{"test"};
  debugPipeline.bindShader("shader");
  debugPipeline.setVertexDescriptions(BlockVertex::getBindingDescription(), BlockVertex::getAttributeDescriptions());
  debugPipeline.setDescriptorLayout(descriptorLayout);
  debugPipeline.setRenderPass(EngineData::i()->vkInstWrapper.renderPass);
  debugPipeline.setPolygonMode(VK_POLYGON_MODE_FILL);
  debugPipeline.build();
  EngineData::i()->vkInstWrapper.globalPipeline = debugPipeline;

  VulkanPipeline::createFramebuffers();

  Commandbuffer::create();

  VulkanPipeline::createSyncObjects();
}

// !!! CHUNK MULTITHREADING HIGHLY WIP !!!
std::vector<Chunk> chunksGenerated;
const int size = 2;

// TODO: At the moment this is not mutexed and will do undefined behaviour!
void generateChunks() {
  FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("EADNzCxAGQATAClcDz4NAAQAAADNzCxAGQAJAAEAAAAAgD8A7FG4PQCF6zlBAQQAAAAAAFK4zkAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACtejPQ==");
  for(int x = -size; x < size; x++) {
    for(int y = -size; y < size; y++) {
      for(int z = -size; z < size; z++) {
        std::vector<float> noise(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
        fnGenerator->GenUniformGrid3D(noise.data(), z * CHUNK_SIZE, y * CHUNK_SIZE, x * CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.01f, 69);
        Chunk chunk{x, y, z};
        chunk.generate(noise);
        if(chunk.isChunkEmpty()) continue; // <- If chunk generated empty
        chunk.regenerateMesh();

        chunk.setChunkGenerated(true);
        chunksGenerated.push_back(chunk);
      }
    }
  }
}

// Initializes the scene
void Voxelate::initScene() {
  Scene mainScene{};

  LOG(I, "FAST NOISE: Supported SIMD Level: " + std::to_string(FastNoise::SUPPORTED_SIMD_LEVELS));

  // Create a separate thread and detach from main thread so rendering doesn't get blocked
  std::thread chunkBuilder(generateChunks);
  chunkBuilder.detach();

  LOG(D, "Settings scenes");

  SceneManager::i()->curScene = mainScene;
  SceneManager::i()->scenesLoaded.push_back(mainScene);
}

void Voxelate::initGui() {
  IMGUI_CHECKVERSION();
  UI::initUserInterface();
}

void Voxelate::update(float deltaTime) {
  cam.update(EngineData::i()->window, deltaTime);

  int x = (int) (cam.position.x - CHUNK_SIZE) / CHUNK_SIZE;
  int y = (int) (cam.position.y - CHUNK_SIZE) / CHUNK_SIZE;
  int z = (int) (cam.position.z - CHUNK_SIZE) / CHUNK_SIZE;

  // !!! UPDATE CHUNKS HIGHLY WIP !!!
  // TODO: Restructure this
  for(Chunk& chunk : chunksGenerated) {
    glm::ivec3* chunkPos = chunk.getPos();
//    if(chunkPos->x == x && chunkPos->y == y && chunkPos->z == z && !chunk.isLoaded()) {
//      chunk.setChunkLoaded(true);
//      SceneManager::i()->curScene.meshesInScene.push_back(chunk.getChunkMesh().mesh);
//    }
    if(chunk.isLoaded()) continue;
    chunk.setChunkLoaded(true);
    SceneManager::i()->curScene.meshesInScene.push_back(chunk.getChunkMesh().mesh);
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

    update(deltaSeconds);

    // Main Entry for rendering the Engine User Interface
    UI::renderMainInterface(cam);
    // ACTUAL RENDERING
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

  if(enableValidation) {
    VulkanDebug::destroyDebugUtilsMessengerEXT(vki.vkInstance, vki.debugMessenger, nullptr);
  }

  vkDestroyInstance(vki.vkInstance, nullptr);

  LOG(I, "Cleaned everything up");
}