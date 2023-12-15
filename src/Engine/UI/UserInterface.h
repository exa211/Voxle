#pragma once

#ifndef VOXELATE_ENGINE_USERINTERFACE_H
#define VOXELATE_ENGINE_USERINTERFACE_H

#include <Engine.h>
#include <Logging/Logger.h>
#include "../VulkanPipeline/Pipeline/GraphicsPipeline.h"
#include <Logging/RenderTimings.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Voxelate.h"

#include <array>

bool displayFrameProfiler = true;
bool displayMainMenuBar = false;

namespace UI {

  void initUserInterface() {
    //This is overkill but in the ImGui demo it's the same
    VkDescriptorPoolSize poolSizes[] = {
      {VK_DESCRIPTOR_TYPE_SAMPLER,                100},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          100},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          100},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   100},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   100},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         100},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         100},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       100}
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = std::size(poolSizes);

    VkDescriptorPool uiDescriptorPool;
    if (vkCreateDescriptorPool(EngineData::i()->vkInstWrapper.device, &poolInfo, nullptr, &uiDescriptorPool) != VK_SUCCESS)
      LOG(F, "Could not create Engine User Interface");

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.Fonts->AddFontFromFileTTF("../res/font/Nunito-Regular.ttf", 16);

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    style.WindowBorderSize = 0;

    ImGui_ImplGlfw_InitForVulkan(EngineData::i()->window, true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = EngineData::i()->vkInstWrapper.vkInstance;
    initInfo.PhysicalDevice = EngineData::i()->vkInstWrapper.physicalDevice;
    initInfo.Device = EngineData::i()->vkInstWrapper.device;
    initInfo.Queue = EngineData::i()->vkInstWrapper.graphicsQueue;
    initInfo.DescriptorPool = uiDescriptorPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, EngineData::i()->vkInstWrapper.renderPass);

    Pipeline::immediateSubmit([&](VkCommandBuffer cmd) {
      ImGui_ImplVulkan_CreateFontsTexture();
    });

    std::string iniData = "";
    ImGui::LoadIniSettingsFromMemory(iniData.c_str(), iniData.size());

    LOG(I, "Created Engine User Interface");
  }

  void renderFrameProfiler() {
    FrameProfiler &profiler = EngineData::i()->frameProfiler;
    ImGui::Begin("Rendering Metrics", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |
                 ImGuiWindowFlags_NoMove);
    {
      ImGui::SetWindowPos(ImVec2(3, 27));
      ImGui::Text("Render Profiler");
      ImGui::TextColored(profiler.timingPerfIndicatorColor(), profiler.metrics().c_str());
    }
    ImGui::End();
  }

  void renderMainMenuBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##MainMenuBarRect01", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImVec2(W_WIDTH, 30));
    {
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      drawList->AddRectFilled(ImVec2(0, 0), ImVec2(W_WIDTH, 30), ImColor(40, 40, 40, 255));
      //drawList->AddQuadFilled(ImVec2(0, 0), ImVec2(W_WIDTH, 0), ImVec2(0, 30), ImVec2(W_WIDTH, 30), ImColor(20, 30, 235, 255));
      ImGui::SetCursorPos(ImVec2(W_WIDTH - 16 - 3, 3));
      if(ImGui::Button("X", ImVec2(16, 16))) {
        glfwSetWindowShouldClose(EngineData::i()->window, true);
      }
      if(ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
      }
      ImGui::SetCursorPos(ImVec2(5, 7));
      ImGui::Text("Voxelate");
    }
    ImGui::Dummy(ImVec2(W_WIDTH, 30));
    ImGui::End();
    ImGui::PopStyleVar(1);

    ImGui::BeginMainMenuBar();
    ImGui::SetWindowPos(ImVec2(120, 0));
    ImGui::MenuItem("File");
    ImGui::SetCursorPos(ImVec2(W_WIDTH - 160, 0));

    if(ImGui::Button("X")) {
      glfwSetWindowShouldClose(EngineData::i()->window, true);
    }
    if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    if(ImGui::Button("_")) {
      glfwIconifyWindow(EngineData::i()->window);
    }
    if(ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::EndMainMenuBar();
  }

  void renderMainInterface(Camera& cam) {

    // Init new frame for ImGui
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    // Debug Panel
    ImGui::NewFrame();
      ImGui::Begin("##DebugPanel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::Text("Camera Position: ");
      ImGui::SameLine();

      std::string camPosStr;
      camPosStr.append("(");
      camPosStr.append(std::to_string(cam.position.x) + ", ");
      camPosStr.append(std::to_string(cam.position.y) + ", ");
      camPosStr.append(std::to_string(cam.position.z) + ")");

      ImGui::Text(camPosStr.c_str());

      ImGui::End();
    ImGui::EndFrame();

    // ---- Start draw commands ImGui ----

//    // Main Menu Bar
//    if(displayMainMenuBar) {
//      renderMainMenuBar();
//    }
//
//    // Frame Profiler
//    if (displayFrameProfiler) {
//      renderFrameProfiler();
//    }

    // ---- End draw commands ImGui ----

    ImGui::Render();

  }
}
#endif