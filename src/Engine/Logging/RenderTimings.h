#pragma once

#include <cassert>

#include <imgui.h>

class FrameProfiler {
public:

  explicit FrameProfiler(float profilingInterval = 0.03f) : profilingInterval(profilingInterval) {
    assert(profilingInterval > 0.0f);
  }

  bool tick(float deltaSeconds) {
    ++numFrames;
    accumulatedTime += deltaSeconds;
    // If we are under the profiling interval don't tick
    if(accumulatedTime < profilingInterval) return false;
    currentFps = static_cast<int>(numFrames/accumulatedTime);
    numFrames = 0;
    accumulatedTime = 0;
    return true;
  }

  [[nodiscard]] inline int fps() const {
    return currentFps;
  }

  [[nodiscard]] inline float ms() const {
    return (float) (1000.0f/(float)currentFps);
  }

  [[nodiscard]] inline std::string metrics() const {
    return std::string(std::to_string(fps()) + " fps | " + std::to_string(ms()).substr(0, 4) + " ms");
  }

  // Returns color indicating if the current performance is bad or good
  [[nodiscard]] inline ImVec4 timingPerfIndicatorColor() const {
    if(ms() < 13) return {92/255.0f, 255/255.0f, 108/255.0f, 1}; // Green
    return {255/255.0f, 92/255.0f, 95/255.0f, 1}; // Red
  }

private:
  const float profilingInterval;
  unsigned int numFrames = 0;
  double accumulatedTime = 0;
  int currentFps = 0;
};
