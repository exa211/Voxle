#include <Voxelate.h>
#include <iostream>

int main() {
    Voxelate voxEngine{};

    try {
        voxEngine.run();
    } catch (std::exception& e) {
        LOG(F, e.what());
    }
    return 0;
}
