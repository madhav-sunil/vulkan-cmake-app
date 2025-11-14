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

    triangleRenderer_ = std::make_unique<TriangleRenderer>(
        vulkanCore.device(),
        vulkanCore.renderPass(),
        vulkanCore.extent()
    );

    return true;  // Return true if initialization is successful
}

void VkApp::run() {
    std::cout << "Entering main loop...\n";
    
    while (!glfwWindowShouldClose(this->getWindow())) {
        glfwPollEvents();
        
        // Draw frame using VulkanCore
        bool ok = this->getVulkanInstance()
                        .drawFrame([&](VkCommandBuffer cmd, uint32_t imageIndex) {
                                triangleRenderer_->recordCommands(cmd);
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
    triangleRenderer_.reset();
    
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}