#pragma once

#include <vector>
#include "Scene.h"
  class SceneManager {

  public:
    Scene curScene; // Current scene rendering
    std::vector<Scene> scenesLoaded;

    static SceneManager *i() {
      static SceneManager instance{};
      return &instance;
    }

    SceneManager(SceneManager const &) = delete;

    void operator=(SceneManager const &) = delete;
  };