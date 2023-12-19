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

class ThreadPool {
public:
  void start(uint32_t threadOverride);
  void addJob(const std::function<void()>& function);
private:
  void ThreadLoop();
  bool shouldTerminate = false;

  std::mutex mainMutex;
  std::condition_variable mainMutexCondition;

  std::vector<std::unique_ptr<std::thread>> threads;
  std::queue<std::function<void()>> functionsQueued;
};