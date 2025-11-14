#pragma once
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

  bool _framebufferResized = false;

  static void framebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) {
    auto app = reinterpret_cast<VkApp *>(glfwGetWindowUserPointer(window));
    app->_framebufferResized = true;
  }
};