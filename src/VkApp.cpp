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
  // initialize GLFW and create a window, then pass it to the VulkanCore
  // initializer
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  _window = glfwCreateWindow(1280, 720, "vk-app", nullptr, nullptr);
  glfwSetWindowUserPointer(_window, this);
  glfwSetFramebufferSizeCallback(_window, framebufferResizeCallback);

  if (!_window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return false;
  }

  // initialize Vulkan via VulkanCore member '_vulkanCore' (ensure VkApp has
  // vulkan::VulkanCore _vulkanCore declared)
  if (!_vulkanCore.initialize(_window)) {
    std::cerr << "Failed to initialize VulkanCore\n";
    glfwDestroyWindow(_window);
    glfwTerminate();
    return false;
  }

  _triangleRenderer = std::make_unique<TriangleRenderer>(
      _vulkanCore.device(), _vulkanCore.renderPass(), _vulkanCore.extent());

  return true; // Return true if initialization is successful
}

void VkApp::run() {
  std::cout << "Entering main loop...\n";

  while (!glfwWindowShouldClose(this->getWindow())) {
    glfwPollEvents();

    if (_framebufferResized) {
      _framebufferResized = false;
      _vulkanCore.waitIdle();

      if (!_vulkanCore.recreateSwapchain()) {
        std::cerr << "Failed to recreate swapchain after resize\n";
        break;
      }
      _triangleRenderer->resize(_vulkanCore.extent());
      std::cout << "Swapchain recreated successfully\n";
    }

    // Draw frame using VulkanCore
    bool ok =
        _vulkanCore.drawFrame([&](VkCommandBuffer cmd, uint32_t imageIndex) {
          _triangleRenderer->recordCommands(cmd);
        });

    if (!ok) {
      _framebufferResized = true;
    }
  }

  std::cout << "Exiting main loop...\n";
}

void VkApp::cleanup() {
  // Cleanup code for Vulkan and the application
  _triangleRenderer.reset();

  if (_window) {
    glfwDestroyWindow(_window);
    _window = nullptr;
  }
  glfwTerminate();
}