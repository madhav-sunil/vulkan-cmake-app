#pragma once
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>

namespace vulkan {

class VulkanSwapchain {
public:
  VulkanSwapchain(VkDevice device, VkPhysicalDevice physicalDevice,
                  VkSurfaceKHR surface, GLFWwindow *window);

  ~VulkanSwapchain();

  auto querySurfaceCapabilities() -> bool;

  // Initialize swapchain
  auto create(VkRenderPass renderPass) -> bool;

  // Recreate swapchain (on resize/out-of-date)
  auto recreate(VkRenderPass renderPass) -> bool;

  // Cleanup current swapchain resources
  auto cleanup() -> void;

  // Accessors
  auto swapchain() const -> const VkSwapchainKHR * { return &_swapchain; }
  auto extent() const -> VkExtent2D { return _extent; }
  auto imageFormat() const -> VkFormat { return _imageFormat; }
  auto imageCount() const -> uint32_t {
    return static_cast<uint32_t>(_images.size());
  }
  auto imageViews() const -> const std::vector<VkImageView> & {
    return _imageViews;
  }
  auto framebuffers() const -> const std::vector<VkFramebuffer> & {
    return _framebuffers;
  }
  auto framebuffer(uint32_t index) const -> VkFramebuffer {
    return _framebuffers[index];
  }

private:
  // Creation helpers
  bool createSwapchain();
  bool createImageViews();
  bool createFramebuffers(VkRenderPass renderPass);

  // Query helpers
  VkSurfaceFormatKHR
  chooseFormat(const std::vector<VkSurfaceFormatKHR> &available);
  VkPresentModeKHR
  choosePresentMode(const std::vector<VkPresentModeKHR> &available);
  VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities);

private:
  VkDevice _device;
  VkPhysicalDevice _physicalDevice;
  VkSurfaceKHR _surface;
  GLFWwindow *_window;

  VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
  VkSwapchainKHR _oldSwapchain = VK_NULL_HANDLE;

  std::vector<VkImage> _images;
  std::vector<VkImageView> _imageViews;
  std::vector<VkFramebuffer> _framebuffers;

  VkFormat _imageFormat = VK_FORMAT_UNDEFINED;
  VkExtent2D _extent{};

  VkSurfaceFormatKHR _chosenFormat{};
  VkPresentModeKHR _chosenPresentMode{};
};

} // namespace vulkan