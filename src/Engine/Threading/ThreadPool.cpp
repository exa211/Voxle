#include "ThreadPool.hpp"

#include <Logging/Logger.h>

#include "Engine.h"

void ThreadPool::LogicThreading() {
  while (true) {
    std::function<void()> currentJob;
    {
      // Lock our mutex
      std::unique_lock<std::mutex> lock(logicMutex);
      // Check mutex condition
      logicMutexCond.wait(lock, [this] {
        return !functionsQueuedLogic.empty() || shouldTerminate;
      });

      // If the thread should terminate, return early.
      if (shouldTerminate) return;

      // Get the last inserted function?
      currentJob = functionsQueuedLogic.front();
      functionsQueuedLogic.pop();
    }
    // Call function pointer
    currentJob();
  }
}

std::mutex builderMutex;
std::condition_variable builderSignal;

void ThreadPool::Builder() {
  static ChunkHandler &chunkHandler = EngineData::i()->chunkHandler;
  std::deque<glm::ivec3> *chunkGenList = chunkHandler.getChunkGenQueue();

  while (true) {
    std::unique_lock<std::mutex> lock(builderMutex);

    // Sleep this thread if chunkGenList is empty or not signaled
    builderSignal.wait(lock, [chunkGenList] {
      return !chunkGenList->empty();
    });

    glm::ivec3 pos = chunkGenList->front();
    chunkGenList->pop_front();

    chunkHandler.generateChunk(pos);
  }

}

void ThreadPool::RenderThreading() {
  while (true) {
    std::function<void()> currentJob;
    {
      // Lock our mutex
      std::unique_lock<std::mutex> lock(renderMutex);
      // Check mutex condition
      renderMutexCond.wait(lock, [this] {
        return !functionsQueuedRender.empty() || shouldTerminate;
      });

      // If the thread should terminate, return early.
      if (shouldTerminate) return;

      // Get the last inserted function?
      currentJob = functionsQueuedRender.front();
      functionsQueuedRender.pop();
    }
    // Call function pointer
    currentJob();
  }
}

void ThreadPool::queueFunction(ThreadType type, const std::function<void()> &function) {
  switch (type) {
    case ThreadType::LOGIC: {
      {
        std::unique_lock<std::mutex> lock(logicMutex);
        functionsQueuedLogic.push(function);
      }
    }
      logicMutexCond.notify_one();
      break;

    case ThreadType::BUILDING: {
      {
        std::unique_lock<std::mutex> lock(meshMutex);
        functionsQueuedMeshing.push(function);
      }
    }
      meshMutexCond.notify_one();
      break;

    case ThreadType::RENDERING: {
      {
        std::unique_lock<std::mutex> lock(renderMutex);
        functionsQueuedRender.push(function);
      }
    }
      renderMutexCond.notify_one();
      break;
  }
}

/**
 * Start the Thread-pool this should only be called once in an engine lifetime.
 **/
void ThreadPool::start(const ThreadSet set, uint32_t threadOverride = 0) {
  uint8_t num_thread_avail = std::thread::hardware_concurrency();

  uint8_t numLogicThreads = set.logicThreads;
  uint8_t numMeshThreads = set.meshThreads;
  uint8_t numRenderThreads = set.renderThreads;
  uint16_t threadSetSum = numLogicThreads + numMeshThreads + numRenderThreads;

  LOG(D, "Threads available: " + std::to_string(num_thread_avail));

  // Viable threadOverride check
  if (threadOverride != 0
      && threadOverride <= num_thread_avail
      && threadOverride <= threadSetSum) {
    num_thread_avail = threadOverride;
  }

  logicThreads.clear();
  meshThreads.clear();
  renderThreads.clear();

  for (uint8_t i = 0; i < numLogicThreads; ++i) {
    logicThreads.push_back(std::make_unique<std::thread>(&ThreadPool::LogicThreading, this));
  }

  for (uint8_t i = 0; i < numMeshThreads; ++i) {
    meshThreads.push_back(std::make_unique<std::thread>(&ThreadPool::Builder, this));
  }

  for (uint32_t i = 0; i < numRenderThreads; ++i) {
    renderThreads.push_back(std::make_unique<std::thread>(&ThreadPool::RenderThreading, this));
  }
}

void ThreadPool::signalBuilderThreads() {
  builderSignal.notify_one();
}
