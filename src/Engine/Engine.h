#pragma once

// Engine
#include "Window/EngineWindow.h"

// Logging / Profiling
#include "Logging/Logger.h"
#include <Logging/RenderTimings.h>

// Rendering
#include <VulkanPipeline/VulkanInstance.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <string>
#include <thread>

class EngineData {
public:
    const std::string ver = "1.0.0";

    std::string title;
    int w_frameBuffer, h_frameBuffer;

    VulkanInstance vkInstWrapper{};
    FrameProfiler frameProfiler{};

    glm::vec2 cursorPos;

    GLFWwindow *window;

    std::thread logicThread;
    std::thread renderThread;

    static EngineData *i() {
      static EngineData instance{};
      return &instance;
    }

    EngineData(EngineData const &) = delete;

    void operator=(EngineData const &) = delete;
};
