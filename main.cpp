#include <Voxelate.h>
#include <iostream>

int main() {
    Voxelate voxEngine{};

    try {
        voxEngine.run();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
