#pragma once
#include <iostream>

namespace LOG {
    inline void info(const std::string& info) {
        std::cout << "\x1B[37m[INFO] " << info << "\033[0m" << std::endl;
    }
    inline void warn(const std::string& warning) {
        std::cout << "\x1B[33m[WARNING] " << warning << "\033[0m" << std::endl;
    }
    inline void fatal(const std::string& fatal) {
        std::cout << "\x1B[91m[FATAL] " << fatal << "\033[0m" << std::endl;
        throw std::runtime_error(fatal);
    }
    inline void ignored(const std::string& ignored) {
        std::cout << "\x1B[90m[IGNORED] " << ignored << "\033[0m" << std::endl;
    }
    inline void important(const std::string& important) {
      std::cout << "\x1B[36m[IMPORTANT] " << important << "\033[0m" << std::endl;
    }
}
