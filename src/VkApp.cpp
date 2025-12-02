#include "VkApp.hpp"
#include "CameraConstants.hpp"
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

  if (!_vulkanCore.initialize(_window)) {
    std::cerr << "Failed to initialize VulkanCore\n";
    glfwDestroyWindow(_window);
    glfwTerminate();
    return false;
  }

  auto extent = _vulkanCore.extent();
  float aspect = extent.width / static_cast<float>(extent.height);

  glm::vec3 cameraPos = CameraConstants::Defaults::FREE_CAMERA_POSITION;

  _camera = std::make_unique<Camera>(aspect, cameraPos);
  _inputSystem = std::make_unique<InputSystem>(_window);
  _inputSystem->enableMouseCapture(false);
  _cameraController = std::make_unique<FreeCameraController>();

  // _triangleRenderer = std::make_unique<TriangleRenderer>(
  //     _vulkanCore.device(), _vulkanCore.renderPass(), _vulkanCore.extent());

  _gridRenderer = std::make_unique<GridRenderer>(
      _vulkanCore.device(), _vulkanCore.renderPass(), _vulkanCore.extent());

  return true;
}

void VkApp::run() {
  std::cout << "Entering main loop...\n";

  float angle = 0.0f;
  float gridScale = 0.1f;

  _lastFrameTime = static_cast<float>(glfwGetTime());
  _deltaTime = 0.0f;

  while (!glfwWindowShouldClose(_window)) {
    float currentTime = glfwGetTime();         // Current time in seconds
    _deltaTime = currentTime - _lastFrameTime; // Time since last frame
    _lastFrameTime = currentTime;              // Store for next frame

    glfwPollEvents();

    // Update input system FIRST
    _inputSystem->update();

    // Handle input actions
    if (_inputSystem->getButtonDown(InputAction::Exit)) {
      glfwSetWindowShouldClose(_window, true);
    }

    if (_inputSystem->getButtonDown(InputAction::ToggleMouseCapture)) {
      _inputSystem->enableMouseCapture(!_inputSystem->isMouseCaptured());
    }

    // Update camera controller
    _cameraController->update(*_camera, *_inputSystem, _deltaTime);

    if (_framebufferResized) {
      _framebufferResized = false;
      _vulkanCore.waitIdle();

      if (!_vulkanCore.recreateSwapchain()) {
        std::cerr << "Failed to recreate swapchain after resize\n";
        break;
      }

      auto extent = _vulkanCore.extent();
      float aspect = extent.width / static_cast<float>(extent.height);
      _camera->updateAspect(aspect);

      _gridRenderer->resize(_vulkanCore.extent());
      std::cout << "Swapchain recreated successfully\n";
    }

    // Update animation
    angle += 0.01f;

    GridPushConstants gridConstants;
    gridConstants.viewProj = _camera->getViewProjectionMatrix();
    gridConstants.invViewProj =
        glm::inverse(_camera->getViewProjectionMatrix());
    gridConstants.cameraPos = _camera->getPosition();
    gridConstants.gridScale = gridScale;

    // Draw frame using VulkanCore
    bool ok =
        _vulkanCore.drawFrame([&](VkCommandBuffer cmd, uint32_t imageIndex) {
          _gridRenderer->recordCommands(cmd, gridConstants); // Draw grid first
          // _triangleRenderer->recordCommands(cmd, camera);  // Then quad on
        });

    if (!ok) {
      _framebufferResized = true;
    }
  }

  std::cout << "Exiting main loop...\n";
}

void VkApp::cleanup() {
  // _triangleRenderer.reset();
  _gridRenderer.reset();
  _cameraController.reset();
  _inputSystem.reset();
  _camera.reset();

  if (_window) {
    glfwDestroyWindow(_window);
    _window = nullptr;
  }
  glfwTerminate();
}