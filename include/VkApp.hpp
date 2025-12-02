#pragma once
#include "Camera.hpp"
#include "CameraController.hpp"
#include "GridRenderer.hpp"
#include "InputSystem.hpp"
#include "TriangleRenderer.hpp"
#include "VulkanCore.hpp"
#include <vulkan/vulkan.h>

class VkApp {
public:
  VkApp();
  ~VkApp();

  bool initialize();
  void run();
  void cleanup();

  auto getWindow() const -> GLFWwindow * { return _window; }
  auto getVulkanInstance() -> vulkan::VulkanCore & { return _vulkanCore; }

private:
  GLFWwindow *_window = nullptr;
  vulkan::VulkanCore _vulkanCore;
  std::unique_ptr<TriangleRenderer> _triangleRenderer;
  std::unique_ptr<GridRenderer> _gridRenderer;

  std::unique_ptr<Camera> _camera;
  std::unique_ptr<InputSystem> _inputSystem;
  std::unique_ptr<CameraController> _cameraController;

  float _lastFrameTime = 0.0f;
  float _deltaTime = 0.0f;

  bool _framebufferResized = false;

  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) {
    auto app = reinterpret_cast<VkApp *>(glfwGetWindowUserPointer(window));
    app->_framebufferResized = true;
  }
};