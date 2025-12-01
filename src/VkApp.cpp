#include "VkApp.hpp"
#include <glm/gtc/matrix_transform.hpp>
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

  // _triangleRenderer = std::make_unique<TriangleRenderer>(
  //     _vulkanCore.device(), _vulkanCore.renderPass(), _vulkanCore.extent());

  _gridRenderer = std::make_unique<GridRenderer>(
      _vulkanCore.device(), _vulkanCore.renderPass(), _vulkanCore.extent());

  return true; // Return true if initialization is successful
}

void VkApp::run() {
  std::cout << "Entering main loop...\n";

  float angle = 0.0f;
  float gridScale = 10.0f;

  while (!glfwWindowShouldClose(this->getWindow())) {
    glfwPollEvents();

    if (_framebufferResized) {
      _framebufferResized = false;
      _vulkanCore.waitIdle();

      if (!_vulkanCore.recreateSwapchain()) {
        std::cerr << "Failed to recreate swapchain after resize\n";
        break;
      }
      _gridRenderer->resize(_vulkanCore.extent());
      std::cout << "Swapchain recreated successfully\n";
    }

    // Update animation
    angle += 0.01f;

    // Calculate camera matrices INSIDE the loop (updates every frame)
    auto extent = _vulkanCore.extent();
    float aspect = extent.width / static_cast<float>(extent.height);

    // Camera position (looking at origin from above and behind)
    glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 10.0f);

    // View matrix (camera transform)
    glm::mat4 view =
        glm::lookAt(cameraPos, // Camera position in world space
                    glm::vec3(0.0f, 0.0f, 0.0f), // Look at origin
                    glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector (Y is up)
        );

    // Projection matrix (perspective)
    glm::mat4 proj =
        glm::perspective(glm::radians(45.0f), // Field of view (45 degrees)
                         aspect,              // Aspect ratio (width/height)
                         0.1f,                // Near clipping plane
                         1000.0f // Far clipping plane (increased for grid)
        );

    // Vulkan clip space correction (inverted Y)
    proj[1][1] *= -1;

    // Combined view-projection matrix
    glm::mat4 viewProj = proj * view;

    // Setup grid push constants
    GridPushConstants gridConstants;
    gridConstants.viewProj = viewProj;
    gridConstants.invViewProj = glm::inverse(viewProj);
    gridConstants.cameraPos = cameraPos;
    gridConstants.gridScale = gridScale;

    // Draw frame using VulkanCore
    // bool ok =
    //     _vulkanCore.drawFrame([&](VkCommandBuffer cmd, uint32_t imageIndex) {
    //       _triangleRenderer->recordCommands(cmd);
    //     });

    bool ok =
        _vulkanCore.drawFrame([&](VkCommandBuffer cmd, uint32_t imageIndex) {
          _gridRenderer->recordCommands(cmd, gridConstants); // Draw grid first
          // _triangleRenderer->recordCommands(cmd, camera);  // Then quad on
          // top
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