#include "vk_app.hpp"

VkApp::VkApp() {
    // initialize members if needed
}

VkApp::~VkApp() {
    // ensure resources are released
    cleanup();
}

bool VkApp::initialize() {
    // Initialization code for Vulkan and the application
    return true;  // Return true if initialization is successful
}

void VkApp::run() {
    // Main loop for the application
}

void VkApp::cleanup() {
    // Cleanup code for Vulkan and the application
}