#pragma once


#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include "vulkan/vulkan_core.h"

namespace VulkanThread {

  /**
   *  @brief Data for a single thread that cant be shared with multiple threads.
   **/
  struct ThreadData {
    VkCommandPool cmdPool;
    VkCommandBuffer cmdBuffer;
  };

}

struct ThreadSet {
  uint8_t logicThreads;
  uint8_t meshThreads;
  uint8_t renderThreads;
};

enum class ThreadType {
  LOGIC,
  BUILDING,
  RENDERING
};

class ThreadPool {
public:

  void start(ThreadSet set, uint32_t threadOverride);

  void signalBuilderThreads();

  void queueFunction(ThreadType type, const std::function<void()>& function);

private:

  void LogicThreading();
  void Builder();
  void RenderThreading();

  bool shouldTerminate = false;

  std::mutex logicMutex;
  std::condition_variable logicMutexCond;

  std::mutex meshMutex;
  std::condition_variable meshMutexCond;

  std::mutex renderMutex;
  std::condition_variable renderMutexCond;

  std::vector<std::unique_ptr<std::thread>> logicThreads;
  std::vector<std::unique_ptr<std::thread>> meshThreads;
  std::vector<std::unique_ptr<std::thread>> renderThreads;

  std::queue<std::function<void()>> functionsQueuedLogic;
  std::queue<std::function<void()>> functionsQueuedMeshing;
  std::queue<std::function<void()>> functionsQueuedRender;
};
