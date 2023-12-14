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

class E_Data {
public:
    const std::string ver = "1.0.0";

    std::string title;
    int w_frameBuffer, h_frameBuffer;

    VulkanInstance vkInstWrapper{};
    FrameProfiler frameProfiler{};

    glm::vec2 cursorPos;

    GLFWwindow *window;

    static E_Data *i() {
      static E_Data instance{};
      return &instance;
    }

    E_Data(E_Data const &) = delete;

    void operator=(E_Data const &) = delete;
};
