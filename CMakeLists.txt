cmake_minimum_required(VERSION 3.27)
project(Voxle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static -o3")

find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw binary_dir EXCLUDE_FROM_ALL)
add_subdirectory(lib/glm)
add_subdirectory(lib/FastNoise2)

## Needed
include_directories(lib/vulkan)
include_directories(lib/stb)
include_directories(lib/FastNoise2/include)

include_directories(src/Engine)

#IMGUI START
set(IMGUI_DIR lib/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)

set(IMGUI_IMPL
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp)
#IMGUI END

add_executable(Voxle
        ${IMGUI_IMPL}
        main.cpp
        src/Engine/Voxelate.cpp
        src/Engine/Voxelate.h
        src/Engine/VulkanPipeline/VulkanInstance.cpp
        src/Engine/VulkanPipeline/VulkanInstance.h
        src/Engine/Logging/Logger.h
        src/Engine/Logging/RenderTimings.h
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
        src/Engine/Texture/Texture.cpp
        src/Engine/Texture/Texture.h
        src/Engine/Resource/ResourceHandler.cpp
        src/Engine/Resource/ResourceHandler.h
        src/Engine/VulkanPipeline/Pipeline/PushConstants/GenericPushConstants.h
        src/Engine/VulkanPipeline/Pipeline/RenderPass.cpp
        src/Engine/VulkanPipeline/Pipeline/RenderPass.h
        src/Engine/VulkanPipeline/Pipeline/Commandbuffer.cpp
        src/Engine/VulkanPipeline/Pipeline/Commandbuffer.h)

target_link_libraries(Voxle PUBLIC ${Vulkan_LIBRARIES} glfw glm FastNoise)
target_compile_definitions(Voxle PUBLIC -DImTextureID=ImU64)
