#include <map>
#include <set>
#include "VkSetup.h"
#include "Queue/QueueHelper.h"
#include "Suitability/SuitabilityChecker.h"
#include "Suitability/SwapchainSuitability.h"
#include "Validation/VulkanValidationLayer.h"

void VkSetup::createVulkanInstance() {
  if (enableValidation && !VulkanValidation::checkValidationLayerSupport())
    LOG::fatal("Using validation layers but found none");

  auto extensions = VulkanValidation::getRequireExtensions();

  VkInstance instance;

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Delta Heart";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Voxelate";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  //Tell Vulkan what we need
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (enableValidation) {
    LOG::warn("Using Vulkan ValidationLayers");
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  //Create the Vulkan instance
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan Instance.");
  }

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> vkExtensions(extensionCount);

  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data());

  std::cout << "Available extensions:\n";
  for (const auto &vkExt: vkExtensions) {
    std::cout << '\t' << vkExt.extensionName << '\n';
  }

  //Set the VkInstance to this
  E_Data::i()->vkInstWrapper = VulkanInstance();
  E_Data::i()->vkInstWrapper.vkInstance = instance;
}

void VkSetup::createSurface() {
  if (glfwCreateWindowSurface(E_Data::i()->vkInstWrapper.vkInstance, E_Data::i()->window, nullptr,
                              &E_Data::i()->vkInstWrapper.surface) == VK_SUCCESS) {
    LOG::info("Created Render surface via glfwCreateWindowSurface.");
  }
}

void VkSetup::selectPhysicalDevice() {
  VkSurfaceKHR surface = E_Data::i()->vkInstWrapper.surface;

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(E_Data::i()->vkInstWrapper.vkInstance, &deviceCount, nullptr);

  if (deviceCount == 0) LOG::fatal("No GPU's detected with Vulkan support.");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(E_Data::i()->vkInstWrapper.vkInstance, &deviceCount, devices.data());

  std::multimap<int, VkPhysicalDevice> cards;

  // Checks every GPU for compatibility
  for (const auto &device: devices) {
    uint32_t rating = 0;

    // IS DEVICE SUITABLE CHECKS
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) rating += 1000;

    rating += deviceProperties.limits.maxImageDimension2D;

    if (!deviceFeatures.geometryShader) rating = 0;
    // Check if the graphics card supports CommandQueueFamilies
    if (!QueueHelper::findQueueFamilies(device, surface).hasBoth()) rating = 0;

    // Checks for the device extensions supported, if an extension is not supported this will score the graphics card as unusable.
    if (!SuitabilityChecker::checkDeviceExtensionSupport(device, deviceExtensions)) rating = 0;

    // Checks if the graphics device supports a swapchain
    SwapChainSupportDetails swapChainSupport = SwapchainSuitability::querySwapChainDetails(device, surface);
    if (swapChainSupport.formats.empty() && swapChainSupport.presentModes.empty()) rating = 0;
    LOG::info("Using physical device, supports SwapChains");

    cards.insert(std::make_pair(rating, device));
  }

  if (cards.rbegin()->first > 0) {
    VkPhysicalDevice deviceSelected = cards.rbegin()->second;
    E_Data::i()->vkInstWrapper.physicalDevice = deviceSelected;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(deviceSelected, &properties);

    LOG::info("Using physical device: " + std::string(properties.deviceName));
  } else {
    LOG::fatal("No GPU's detected with suitable API requirements.");
  }
}

void VkSetup::createLogicalDevice() {
  VkPhysicalDevice &physicalDevice = E_Data::i()->vkInstWrapper.physicalDevice;
  VkSurfaceKHR &surface = E_Data::i()->vkInstWrapper.surface;
  VkDevice device = nullptr;

  if (physicalDevice == nullptr) LOG::fatal("No physical device is set");
  QueueFamilyIndices indices = QueueHelper::findQueueFamilies(physicalDevice, surface);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  //Set the queue execution priority
  float queuePrio = 1.0f;

  for (uint32_t queueFamily: uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePrio;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  // Create Logical Device Info
  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  createInfo.pQueueCreateInfos = queueCreateInfos.data();
  createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

  createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();
  createInfo.pEnabledFeatures = &deviceFeatures;

  // Logical Device creation
  if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    LOG::fatal("Failed to create a Logical Device");

  // Create Device Queues
  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &E_Data::i()->vkInstWrapper.graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &E_Data::i()->vkInstWrapper.presentQueue);

  E_Data::i()->vkInstWrapper.device = device;
  LOG::info("Created Device Queues");
}

void VkSetup::createSwapchain(bool isResize) {
  VkPhysicalDevice &physicalDevice = E_Data::i()->vkInstWrapper.physicalDevice;
  VkSurfaceKHR &surface = E_Data::i()->vkInstWrapper.surface;
  VkDevice &device = E_Data::i()->vkInstWrapper.device;

  int w_frameBuffer = 0;
  int h_frameBuffer = 0;

  glfwGetFramebufferSize(E_Data::i()->window, &w_frameBuffer, &h_frameBuffer);

  VkSwapchainKHR swapChain = nullptr;

  std::vector<VkImage> &swapChainImages = E_Data::i()->vkInstWrapper.swapChainImages;

  SwapChainSupportDetails swapChainSupport = SwapchainSuitability::querySwapChainDetails(physicalDevice, surface);

  // Chooses the best surface formats/present modes/extents available
  VkSurfaceFormatKHR surfaceFormat = SwapchainSuitability::chooseSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = SwapchainSuitability::choosePresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = SwapchainSuitability::chooseExtent(swapChainSupport.caps, w_frameBuffer, h_frameBuffer);

  uint32_t imageQueryCount = swapChainSupport.caps.minImageCount + 1;

  if (swapChainSupport.caps.maxImageCount > 0 && imageQueryCount > swapChainSupport.caps.maxImageCount) {
    imageQueryCount = swapChainSupport.caps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swapCreateInfo{};
  swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapCreateInfo.surface = surface;
  swapCreateInfo.minImageCount = imageQueryCount;
  swapCreateInfo.imageFormat = surfaceFormat.format;
  swapCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
  swapCreateInfo.imageExtent = extent;
  swapCreateInfo.imageArrayLayers = 1;
  swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = QueueHelper::findQueueFamilies(physicalDevice, surface);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
    swapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapCreateInfo.queueFamilyIndexCount = 2;
    swapCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapCreateInfo.queueFamilyIndexCount = 0;
    swapCreateInfo.pQueueFamilyIndices = nullptr;
  }

  swapCreateInfo.preTransform = swapChainSupport.caps.currentTransform;
  swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  swapCreateInfo.presentMode = presentMode;
  swapCreateInfo.clipped = VK_TRUE;

  swapCreateInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(device, &swapCreateInfo, nullptr, &swapChain) != VK_SUCCESS)
    LOG::fatal("Could not create Swapchain");
  if (!isResize) LOG::info("Created Swapchain");

  vkGetSwapchainImagesKHR(device, swapChain, &imageQueryCount, nullptr);
  swapChainImages.resize(imageQueryCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageQueryCount, swapChainImages.data());
  if (!isResize) LOG::info("Resized std::vector<VkImage> swapChainImages");

  E_Data::i()->vkInstWrapper.extent = extent;
  E_Data::i()->vkInstWrapper.format = surfaceFormat.format;
  E_Data::i()->vkInstWrapper.swapChain = swapChain;
  if (!isResize) LOG::info("Set VkExtent2D and VkFormat in VulkanInstance");
}

void VkSetup::createImageViews() {
  std::vector<VkImage> &swapChainImages = E_Data::i()->vkInstWrapper.swapChainImages;
  std::vector<VkImageView> &swapChainImageViews = E_Data::i()->vkInstWrapper.swapChainImageViews;

  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    swapChainImageViews[i] = VulkanImage::createImageView(swapChainImages[i], E_Data::i()->vkInstWrapper.format);
  }

  LOG::info("Created Image Views");
}

void VkSetup::recreateSwapchain(VkDevice &device) {
  int w = 0, h = 0;
  glfwGetFramebufferSize(E_Data::i()->window, &w, &h);
  while (w == 0 || h == 0) {
    glfwGetFramebufferSize(E_Data::i()->window, &w, &h);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(device);

  cleanupOldSwapchain(device);

  createSwapchain(true);
  createImageViews();
  Pipeline::createFramebuffers();
}

void VkSetup::cleanupOldSwapchain(VkDevice &device) {
  for (size_t i = 0; i < E_Data::i()->vkInstWrapper.swapChainFramebuffers.size(); ++i) {
    vkDestroyFramebuffer(device, E_Data::i()->vkInstWrapper.swapChainFramebuffers[i], nullptr);
  }

  for (size_t i = 0; i < E_Data::i()->vkInstWrapper.swapChainImageViews.size(); ++i) {
    vkDestroyImageView(device, E_Data::i()->vkInstWrapper.swapChainImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(device, E_Data::i()->vkInstWrapper.swapChain, nullptr);
}

VkDescriptorSetLayout VkSetup::createDescriptorSetLayout() {
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 1;
  layoutInfo.pBindings = &uboLayoutBinding;

  if (vkCreateDescriptorSetLayout(E_Data::i()->vkInstWrapper.device, &layoutInfo, nullptr, &descriptorSetLayout) !=
      VK_SUCCESS)
    LOG::fatal("Could not create VkDescriptorSetLayout");
  LOG::info("Created VkDescriptorSetLayout");

  E_Data::i()->vkInstWrapper.descriptorSetLayout = descriptorSetLayout;
  return descriptorSetLayout;
}

void VkSetup::createDescriptorPool() {
  auto count = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  VkDescriptorPoolSize poolSize{};
  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSize.descriptorCount = count;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;
  poolInfo.maxSets = count;

  if (vkCreateDescriptorPool(E_Data::i()->vkInstWrapper.device, &poolInfo, nullptr,
                             &E_Data::i()->vkInstWrapper.descriptorPool) != VK_SUCCESS)
    LOG::fatal("Could not create VkDescriptorPool");
}

void VkSetup::createDescriptorSets() {
  Buffer::createUniformBuffers();

  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, E_Data::i()->vkInstWrapper.descriptorSetLayout);

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = E_Data::i()->vkInstWrapper.descriptorPool;
  allocInfo.pSetLayouts = layouts.data();
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  std::cout << MAX_FRAMES_IN_FLIGHT << std::endl;

  E_Data::i()->vkInstWrapper.descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(E_Data::i()->vkInstWrapper.device, &allocInfo,
                               E_Data::i()->vkInstWrapper.descriptorSets.data()) != VK_SUCCESS)
    LOG::fatal("Could not create VkDescriptorSets");
  LOG::info("Created VkDescriptorSets");
}

void VkSetup::populateDescriptors() {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = E_Data::i()->vkInstWrapper.uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = E_Data::i()->vkInstWrapper.descriptorSets[i];
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;
    vkUpdateDescriptorSets(E_Data::i()->vkInstWrapper.device, 1, &descriptorWrite, 0, nullptr);

  }
}
