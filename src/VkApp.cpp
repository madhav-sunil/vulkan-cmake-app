#include "VkApp.hpp"
#include <iostream>

VkApp::VkApp() {
    // initialize members if needed
}

VkApp::~VkApp() {
    // ensure resources are released
    cleanup();
}

bool VkApp::initialize() {
    // initialize GLFW and create a window, then pass it to the VulkanCore initializer
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // store window in member 'window_' (ensure VkApp has GLFWwindow* window_ declared)
    window = glfwCreateWindow(1280, 720, "vk-app", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    // initialize Vulkan via VulkanCore member 'core_' (ensure VkApp has vulkan::VulkanCore core_ declared)
    if (!vulkanCore.initialize(window)) {
        std::cerr << "Failed to initialize VulkanCore\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    return true;  // Return true if initialization is successful
}

void VkApp::run() {
    std::cout << "Entering main loop...\n";
    
    while (!glfwWindowShouldClose(this->getWindow())) {
        glfwPollEvents();
        
        // Draw frame using VulkanCore
        bool ok = this->getVulkanInstance()
                        .drawFrame([&](VkCommandBuffer cmd, uint32_t imageIndex) {
            // Empty for now - just clear screen
            // Grid rendering will go here later
        });
        
        if (!ok) {
            std::cerr << "Draw frame failed - likely swapchain out of date\n";
            // In production, recreate swapchain here
            break;
        }
    }
    
    std::cout << "Exiting main loop...\n";
}

void VkApp::cleanup() {
    // Cleanup code for Vulkan and the application
}