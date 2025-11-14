#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {

class VulkanCore {
public:
  VulkanCore() = default;
  ~VulkanCore();

  // Initialize with an already-created GLFW window
  // returns false on failure
  bool initialize(GLFWwindow *window);

  // Cleanup resources
  void cleanup();

  // Simple frame loop helper:
  // recordFunc is called inside an active renderpass with (cmdBuffer,
  // imageIndex)
  bool
  drawFrame(const std::function<void(VkCommandBuffer, uint32_t)> &recordFunc);

  // Accessors for renderers
  auto device() const -> VkDevice { return _device; }
  auto physicalDevice() const -> VkPhysicalDevice { return _physicalDevice; }
  auto extent() const -> VkExtent2D { return _extent; }
  auto renderPass() const -> VkRenderPass { return _renderPass; }
  auto descriptorPool() const -> VkDescriptorPool { return _descriptorPool; }
  auto commandPool() const -> VkCommandPool { return _commandPool; }
  auto graphicsQueue() const -> VkQueue { return _graphicsQueue; }
  auto swapchainImageFormat() const -> VkFormat {
    return _swapchainImageFormat;
  }

private:
  // init steps
  bool createInstance();
  bool createSurface(GLFWwindow *window);
  bool pickPhysicalDevice();
  bool createLogicalDevice();
  bool createSwapchain();
  bool createImageViews();
  bool createRenderPass();
  bool createFramebuffers();
  bool createCommandPoolAndBuffers();
  bool createSyncObjects();
  bool createDescriptorPool();

  // helpers
  auto chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avail)
      -> VkSurfaceFormatKHR;
  auto choosePresentMode(const std::vector<VkPresentModeKHR> &avail)
      -> VkPresentModeKHR;
  auto chooseExtent(const VkSurfaceCapabilitiesKHR &caps) -> VkExtent2D;

  auto checkValidationLayerSupport() const -> bool;
  auto setupDebugMessenger() -> void;
  auto destroyDebugMessenger() -> void;

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);

private:
#ifdef NDEBUG
  const bool enableValidationLayers_ = false;
#else
  const bool enableValidationLayers_ = true;
#endif

  VkInstance _instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
  VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
  VkDevice _device = VK_NULL_HANDLE;
  VkQueue _graphicsQueue = VK_NULL_HANDLE;
  VkQueue _presentQueue = VK_NULL_HANDLE;
  VkSurfaceKHR _surface = VK_NULL_HANDLE;

  uint32_t _graphicsFamily = UINT32_MAX; // store graphics queue family index
  uint32_t _presentFamily = UINT32_MAX;  // (optional, for clarity)

  VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkFormat _swapchainImageFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D _extent{800, 600};

  VkRenderPass _renderPass = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> _framebuffers;

  VkCommandPool _commandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> _commandBuffers;

  // sync
  std::vector<VkSemaphore> _imageAvailable;
  std::vector<VkSemaphore> _renderFinished;
  std::vector<VkFence> _inFlightFences;
  size_t _currentFrame = 0;
  const size_t MAX_FRAMES_IN_FLIGHT = 2;

  VkDescriptorPool _descriptorPool = VK_NULL_HANDLE;
};
} // namespace vulkan