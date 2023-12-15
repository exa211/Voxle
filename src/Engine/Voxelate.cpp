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
  int cX = (vidMode->width / 2) - W_WIDTH / 2;
  int cY = (vidMode->height / 2) - W_HEIGHT / 2;

  glfwSetWindowPos(EngineData::i()->window, cX, cY);

  glfwGetFramebufferSize(EngineData::i()->window, &EngineData::i()->w_frameBuffer, &EngineData::i()->h_frameBuffer);
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
  Resources::createTexture(leaveTexture, "../res/texture/andesite.png");

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

// Initializes the scene
void Voxelate::initScene() {
  Scene mainScene{};

  LOG(I, "FAST NOISE: Supported SIMD Level: " + std::to_string(FastNoise::SUPPORTED_SIMD_LEVELS));

  FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("EQACAAAA16PwPxAAAAAAQBkAEwDD9Sg/DQAEAAAAAAAgQAkAAGZmJj8AAAAAPwEEAAAAAAD2KIxAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAM3MTD4AMzMzPwAAAAA/");

  std::vector<float> noise(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
  fnGenerator->GenUniformGrid3D(noise.data(), 0, 0, 0, CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE, 0.01f, 0);

  Chunk chunk{};
  chunk.generate(noise);
  chunk.regenerateMesh();

  std::cout << "Chunk Vert Count: " << chunk.getChunkMesh().vertices.size() << std::endl;
  std::cout << "Chunk Index Count: " << chunk.getChunkMesh().indices.size() << std::endl;

  Mesh combinedMesh{};
  combinedMesh.vertexBuffer = Buffer::createBlockVertexBuffer(chunk.getChunkMesh().vertices);
  combinedMesh.indexBuffer = Buffer::createIndexBuffer(chunk.getChunkMesh().indices);
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
  cam.update(EngineData::i()->window, deltaTime);
}

void Voxelate::loop() {
  // RENDER PROFILING
  float deltaSeconds = 0.0f;
  double timeStamp = glfwGetTime();

  /*
   *  LOOP
   */

  while (!glfwWindowShouldClose(EngineData::i()->window)) {
    const double newTimeStamp = glfwGetTime();
    deltaSeconds = (float) (newTimeStamp - timeStamp);
    timeStamp = newTimeStamp;
    EngineData::i()->frameProfiler.tick(deltaSeconds);

    //float ms = 1000/time;

    glfwPollEvents();

    /*
   *  UPDATE
   */
    update(deltaSeconds);

    // Future dockspace layout
    //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    // Main Entry for rendering the Engine User Interface
    UI::renderMainInterface(cam);

    //ImGui::ShowDemoWindow();

    //ImGui::ShowMetricsWindow();

    // ACTUAL RENDERING
    PrimitiveRenderer::render(cam);
  }

  vkDeviceWaitIdle(EngineData::i()->vkInstWrapper.device);

}

void Voxelate::clean() {
  //TODO: Destroy everything else """ATM""" THIS SHOULD BE OK BECAUSE WINDOWS CLEANS MEMORY AFTER AN EXE WAS CLOSED
  VulkanInstance &vki = EngineData::i()->vkInstWrapper;
  VkDevice &device = vki.device;

  // Cleanup Depth Buffering Resources
  vkDestroyImageView(device, EngineData::i()->vkInstWrapper.depthImage.vkImageView, nullptr);
  vkDestroyImage(device, EngineData::i()->vkInstWrapper.depthImage.vkImage, nullptr);
  vkFreeMemory(device, EngineData::i()->vkInstWrapper.depthImage.vkImageMemory, nullptr);

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