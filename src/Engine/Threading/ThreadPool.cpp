#include "ThreadPool.hpp"

#include <Logging/Logger.h>

void ThreadPool::ThreadLoop() {
  while(true) {
    std::function<void()> currentJob;
    {
      // Lock our mutex
      std::unique_lock<std::mutex> lock(mainMutex);
      // Check mutex condition
      mainMutexCondition.wait(lock, [this] {
        return !functionsQueued.empty() || shouldTerminate;
      });

      // If the thread should terminate, return early.
      if(shouldTerminate) return;

      // Get the last inserted function?
      currentJob = functionsQueued.front();
      functionsQueued.pop();
    }
    // Call function pointer
    currentJob();

  }
}

void ThreadPool::addJob(const std::function<void()>& function) {
  {
    std::unique_lock<std::mutex> lock(mainMutex);
    functionsQueued.push(function);
  }
  mainMutexCondition.notify_one();
}

/**
 * Start the Thread-pool this should only be called once in an engine lifetime.
 **/
void ThreadPool::start(uint32_t threadOverride = 0) {
  uint32_t  num_thread_avail = std::thread::hardware_concurrency() - 1; // Spare one thread for rendering

  LOG(D, "Threads available: " + std::to_string(num_thread_avail));

  if(threadOverride != 0 && threadOverride <= num_thread_avail) {
    num_thread_avail = threadOverride;
  }

  LOG(D, "Threads using: " + std::to_string(num_thread_avail));

  threads.clear();
  for(uint32_t i = 0; i < num_thread_avail; ++i) {
    threads.push_back(std::make_unique<std::thread>(&ThreadPool::ThreadLoop, this));
  }
}
