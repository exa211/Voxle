#pragma once

// Logging / Profiling
#include "Logging/Logger.h"
#include <Logging/RenderTimings.h>

// Rendering
#include <VulkanPipeline/VulkanInstance.h>
#include <glm/glm.hpp>

#include <Threading/ThreadPool.hpp>
#include <World/ChunkHandler.hpp>

#include <string>
#include <thread>

class EngineData {
public:
    const std::string ver = "Voxle - 0.1";

    std::string title;
    int w_frameBuffer, h_frameBuffer;

    VulkanInstance vkInstWrapper{};
    FrameProfiler frameProfiler{};

    glm::vec2 cursorPos;

    GLFWwindow *window;

    ThreadPool threadPool{};

    ChunkHandler chunkHandler{};

    static EngineData *i() {
      static EngineData instance{};
      return &instance;
    }

    EngineData(EngineData const &) = delete;

    void operator=(EngineData const &) = delete;
};
