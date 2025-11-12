#include <iostream>
#include "VkApp.hpp"

int main() {
    VkApp app;

    if (!app.initialize()) {
        std::cerr << "Failed to initialize the application." << std::endl;
        return EXIT_FAILURE;
    }

    app.run();
    app.cleanup();

    return EXIT_SUCCESS;
}