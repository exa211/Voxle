#include <map>
#include <set>
#include "VkSetup.h"
#include "Queue/QueueHelper.h"
#include "Suitability/SuitabilityChecker.h"
#include "Suitability/SwapchainSuitability.h"
#include "Validation/VulkanValidationLayer.h"
#include "VulkanDebug.h"

void VkSetup::createVulkanInstance() {
  if (!VulkanValidation::checkValidationLayerSupport())
    LOG(F, "Using validation layers but found none");

  auto extensions = VulkanValidation::getRequireExtensions();

  VkInstance instance{};

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Delta Heart";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Voxelate";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  //Tell Vulkan what we need
  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (enableValidation) {
    LOG(I, "Using Vulkan ValidationLayers");
    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  std::cout << extensions.size() << std::endl;

  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> vkExtensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data());

    std::cout << "Available extensions:\n";
    for (const auto &vkExt: vkExtensions) {
        std::cout << '\t' << vkExt.extensionName << '\n';
    }

  //Create the Vulkan instance
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      LOG(F, "Failed to create Vulkan Instance. (VkSetup.cpp)");
  }

  //Set the VkInstance to this
  E_Data::i()->vkInstWrapper = VulkanInstance();
  E_Data::i()->vkInstWrapper.vkInstance = instance;
}

/**
 *  Creates a VkDebugMessenger so we can get performance leaks
 *  and other stuff from vulkan.
 **/
void VkSetup::createDebugMessenger() {
  if(!enableValidation) return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

  // Next 2 lines is essentially what is going to be logged
  // In short terms messages will be filtered
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

  createInfo.pfnUserCallback = VulkanDebug::debugCallback; // <- static debugCallback()
  createInfo.pUserData = nullptr;

  if(VulkanDebug::createDebugUtilsMessengerEXT(E_Data::i()->vkInstWrapper.vkInstance,
                                               &createInfo,
                                               nullptr,
                                               &E_Data::i()->vkInstWrapper.debugMessenger) != VK_SUCCESS) {
    LOG(W, "Could not create VkDebugMessenger skipping ahead.");
  }
}

void VkSetup::createSurface() {
  if (glfwCreateWindowSurface(E_Data::i()->vkInstWrapper.vkInstance, E_Data::i()->window, nullptr,
                              &E_Data::i()->vkInstWrapper.surface) == VK_SUCCESS) {
    LOG(I, "Created Render surface via glfwCreateWindowSurface.");
  }
}

void VkSetup::selectPhysicalDevice() {
  VkSurfaceKHR surface = E_Data::i()->vkInstWrapper.surface;

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(E_Data::i()->vkInstWrapper.vkInstance, &deviceCount, nullptr);

  if (deviceCount == 0) LOG(F, "No GPU's detected with Vulkan support.");

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
    LOG(I, "Using physical device, supports SwapChains");

    cards.insert(std::make_pair(rating, device));
  }

  if (cards.rbegin()->first > 0) {
    VkPhysicalDevice deviceSelected = cards.rbegin()->second;
    E_Data::i()->vkInstWrapper.physicalDevice = deviceSelected;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(deviceSelected, &properties);

    LOG(I, "Using physical device: " + std::string(properties.deviceName));
  } else {
    LOG(F, "No GPU's detected with suitable API requirements.");
  }
}

void VkSetup::createLogicalDevice() {
  VkPhysicalDevice &physicalDevice = E_Data::i()->vkInstWrapper.physicalDevice;
  VkSurfaceKHR &surface = E_Data::i()->vkInstWrapper.surface;
  VkDevice device = nullptr;

  if (physicalDevice == nullptr) LOG(F, "No physical device is set");
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

  // Query our device features so we can enable them
  VkPhysicalDeviceFeatures deviceFeaturesStruct;
  vkGetPhysicalDeviceFeatures(E_Data::i()->vkInstWrapper.physicalDevice, &deviceFeaturesStruct);

  // Enable device features
  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = deviceFeaturesStruct.samplerAnisotropy;

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
    LOG(F, "Failed to create a Logical Device");

  // Create Device Queues
  vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &E_Data::i()->vkInstWrapper.graphicsQueue);
  vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &E_Data::i()->vkInstWrapper.presentQueue);

  E_Data::i()->vkInstWrapper.device = device;
  LOG(I, "Created Device Queues");
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
    LOG(F, "Could not create Swapchain");
  if (!isResize) LOG(I, "Created Swapchain");

  vkGetSwapchainImagesKHR(device, swapChain, &imageQueryCount, nullptr);
  swapChainImages.resize(imageQueryCount);
  vkGetSwapchainImagesKHR(device, swapChain, &imageQueryCount, swapChainImages.data());
  if (!isResize) LOG(I, "Resized std::vector<VkImage> swapChainImages");

  E_Data::i()->vkInstWrapper.extent = extent;
  E_Data::i()->vkInstWrapper.format = surfaceFormat.format;
  E_Data::i()->vkInstWrapper.swapChain = swapChain;
  if (!isResize) LOG(I, "Set VkExtent2D and VkFormat in VulkanInstance");
}

void VkSetup::createImageViews() {
  std::vector<VkImage> &swapChainImages = E_Data::i()->vkInstWrapper.swapChainImages;
  std::vector<VkImageView> &swapChainImageViews = E_Data::i()->vkInstWrapper.swapChainImageViews;

  swapChainImageViews.resize(swapChainImages.size());

  for (size_t i = 0; i < swapChainImages.size(); i++)
    VulkanImage::createImageView(swapChainImages[i], swapChainImageViews[i], E_Data::i()->vkInstWrapper.format);

  LOG(I, "Created Image Views");
}

void VkSetup::createSampler() {
  LOG(I, "Created Main Texture Sampler (VkSetup.cpp)");
  VulkanImage::Sampler::createTextureSampler(E_Data::i()->vkInstWrapper.mainSampler, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
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
  Pipeline::createDepthBufferingObjects();
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

  // Uniform buffer object layout binding
  VkDescriptorSetLayoutBinding uboLayoutBinding{};
  uboLayoutBinding.binding = 0;
  uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboLayoutBinding.descriptorCount = 1;
  uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  uboLayoutBinding.pImmutableSamplers = nullptr;

  // TextureSampler layout binding
  VkDescriptorSetLayoutBinding samplerLayoutBinding{};
  samplerLayoutBinding.binding = 1;
  samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  samplerLayoutBinding.descriptorCount = 1;
  samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  samplerLayoutBinding.pImmutableSamplers = nullptr;

  std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<int>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  VkDescriptorSetLayout descriptorSetLayout;

  if (vkCreateDescriptorSetLayout(E_Data::i()->vkInstWrapper.device, &layoutInfo, nullptr, &descriptorSetLayout) !=
      VK_SUCCESS)
    LOG(F, "Could not create VkDescriptorSetLayout");
  LOG(I, "Created VkDescriptorSetLayout");

  E_Data::i()->vkInstWrapper.descriptorSetLayout = descriptorSetLayout;
  return descriptorSetLayout;
}

void VkSetup::createDescriptorPool() {
  auto count = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  std::array<VkDescriptorPoolSize, 2> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = count;
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = count;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<int>(poolSizes.size());
  poolInfo.maxSets = static_cast<int>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();

  if (vkCreateDescriptorPool(E_Data::i()->vkInstWrapper.device, &poolInfo, nullptr,
                             &E_Data::i()->vkInstWrapper.descriptorPool) != VK_SUCCESS)
    LOG(F, "Could not create VkDescriptorPool");
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
    LOG(F, "Could not create VkDescriptorSets");
  LOG(I, "Created VkDescriptorSets");
}

void VkSetup::populateDescriptors(VulkanImage::InternalImage& image) {
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = E_Data::i()->vkInstWrapper.uniformBuffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = image.vkImageView;
    imageInfo.sampler = E_Data::i()->vkInstWrapper.mainSampler;

    std::array<VkWriteDescriptorSet, 2> writes{};

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = E_Data::i()->vkInstWrapper.descriptorSets[i];
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &bufferInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = E_Data::i()->vkInstWrapper.descriptorSets[i];
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].descriptorCount = 1;
    writes[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(E_Data::i()->vkInstWrapper.device, static_cast<int>(writes.size()), writes.data(), 0, nullptr);
  }
}
