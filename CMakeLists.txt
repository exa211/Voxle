cmake_minimum_required(VERSION 3.27)
project(Voxle)

set(CMAKE_CXX_STANDARD 17)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentatVulkanPipelineion" OFF)

add_subdirectory(lib/FastNoise2)
add_subdirectory(lib/glfw)
add_subdirectory(lib/glm)
add_subdirectory(lib/VulkanMemoryAllocator)

add_compile_definitions(GLFW_INCLUDE_NONE)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)

include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends)

set(IMGUI_SRC
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp)

include_directories(src/Engine)
include_directories(lib/glfw/deps)

add_executable(Voxle
        ${IMGUI_SRC}
        src/Engine/Voxelate.cpp
        src/Engine/Voxelate.h
        src/Engine/VulkanPipeline/VulkanInstance.cpp
        src/Engine/VulkanPipeline/VulkanInstance.h
        #src/Logging/Logger.h
        src/Engine/Engine.h
        src/Engine/VulkanPipeline/VkSetup.cpp
        src/Engine/VulkanPipeline/VkSetup.h
        src/Engine/VulkanPipeline/Queue/QueueHelper.h
        src/Engine/VulkanPipeline/Queue/QueueHelper.cpp
        src/Engine/VulkanPipeline/Suitability/SuitabilityChecker.cpp
        src/Engine/VulkanPipeline/Suitability/SuitabilityChecker.h
        src/Engine/VulkanPipeline/Suitability/SwapchainSuitability.cpp
        src/Engine/VulkanPipeline/Suitability/SwapchainSuitability.h
        src/Engine/VulkanPipeline/Pipeline/GraphicsPipeline.cpp
        src/Engine/VulkanPipeline/Pipeline/GraphicsPipeline.h
        src/Engine/Shader/Shader.h
        src/Engine/Shader/Shader.cpp
        src/Engine/VulkanPipeline/Validation/VulkanValidationLayer.h
        src/Engine/Renderer/PrimitiveRenderer.cpp
        src/Engine/Renderer/PrimitiveRenderer.h
        src/Engine/Renderer/Primitives/MeshPrimitives.h
        src/Engine/VulkanPipeline/Pipeline/Buffer/Buffer.h
        src/Engine/Renderer/Mesh/Mesh.cpp
        src/Engine/Renderer/Mesh/Mesh.h
        src/Engine/Scene/Scene.cpp
        src/Engine/Scene/Scene.h
        src/Engine/VulkanPipeline/Pipeline/Buffer/Buffer.cpp
        src/Engine/UI/UserInterface.h
        src/Engine/Scene/SceneManager.h
        src/Engine/Window/EngineWindow.h
        src/Engine/Color/Color.h
        src/Engine/Camera/Camera.cpp
        src/Engine/Camera/Camera.h
        #src/Logging/RenderTimings.h
        src/Engine/Texture/Texture.cpp
        src/Engine/Texture/Texture.h
        src/Engine/Resource/ResourceHandler.cpp
        src/Engine/Resource/ResourceHandler.h
        src/Engine/VulkanPipeline/Pipeline/PushConstants/GenericPushConstants.h
        src/Engine/VulkanPipeline/Pipeline/RenderPass.cpp
        src/Engine/VulkanPipeline/Pipeline/RenderPass.h
        src/Engine/VulkanPipeline/Pipeline/Commandbuffer.cpp
        src/Engine/VulkanPipeline/Pipeline/Commandbuffer.h)

target_link_libraries(Voxle glfw glm)
