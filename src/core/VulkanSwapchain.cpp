#include "VulkanSwapchain.hpp"
#include <algorithm>
#include <iostream>
#include <limits>

using namespace vulkan;

VulkanSwapchain::VulkanSwapchain(VkDevice device,
                                 VkPhysicalDevice physicalDevice,
                                 VkSurfaceKHR surface, GLFWwindow *window)
    : _device(device), _physicalDevice(physicalDevice), _surface(surface),
      _window(window) {
  querySurfaceCapabilities();
}

VulkanSwapchain::~VulkanSwapchain() { cleanup(); }

auto VulkanSwapchain::querySurfaceCapabilities() -> bool {
  // Query formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
                                       nullptr);
  if (formatCount == 0) {
    std::cerr << "No surface formats available\n";
    return false;
  }
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
                                       formats.data());

  // Query present modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface,
                                            &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      _physicalDevice, _surface, &presentModeCount, presentModes.data());

  // Choose and cache settings
  _chosenFormat = chooseFormat(formats);
  _chosenPresentMode = choosePresentMode(presentModes);
  _imageFormat = _chosenFormat.format; // Expose format early

  std::cout << "Swapchain format selected: " << _imageFormat << "\n";
  return true;
}

bool VulkanSwapchain::create(VkRenderPass renderPass) {
  if (!createSwapchain())
    return false;
  if (!createImageViews())
    return false;
  if (!createFramebuffers(renderPass))
    return false;
  return true;
}

bool VulkanSwapchain::recreate(VkRenderPass renderPass) {
  // Wait for window to have valid size (handle minimization)
  int width = 0, height = 0;
  glfwGetFramebufferSize(_window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(_window, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(_device);

  // Store old swapchain for recreation
  _oldSwapchain = _swapchain;

  // Clean up dependent resources
  for (auto fb : _framebuffers) {
    vkDestroyFramebuffer(_device, fb, nullptr);
  }
  _framebuffers.clear();

  for (auto iv : _imageViews) {
    vkDestroyImageView(_device, iv, nullptr);
  }
  _imageViews.clear();
  _images.clear();

  // Recreate swapchain and resources
  if (!createSwapchain())
    return false;
  if (!createImageViews())
    return false;
  if (!createFramebuffers(renderPass))
    return false;

  // Destroy old swapchain after new one is created
  if (_oldSwapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(_device, _oldSwapchain, nullptr);
    _oldSwapchain = VK_NULL_HANDLE;
  }

  return true;
}

void VulkanSwapchain::cleanup() {
  for (auto fb : _framebuffers) {
    vkDestroyFramebuffer(_device, fb, nullptr);
  }
  _framebuffers.clear();

  for (auto iv : _imageViews) {
    vkDestroyImageView(_device, iv, nullptr);
  }
  _imageViews.clear();

  if (_swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
    _swapchain = VK_NULL_HANDLE;
  }
}

bool VulkanSwapchain::createSwapchain() {
  // Query surface capabilities
  VkSurfaceCapabilitiesKHR capabilities;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface,
                                            &capabilities);

  // Query formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
                                       nullptr);
  if (formatCount == 0) {
    std::cerr << "No surface formats available\n";
    return false;
  }
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
                                       formats.data());

  // Query present modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface,
                                            &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      _physicalDevice, _surface, &presentModeCount, presentModes.data());

  // Choose settings
  VkSurfaceFormatKHR surfaceFormat = chooseFormat(formats);
  VkPresentModeKHR presentMode = choosePresentMode(presentModes);
  VkExtent2D extent = chooseExtent(capabilities);

  // Image count
  uint32_t imageCount = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 &&
      imageCount > capabilities.maxImageCount) {
    imageCount = capabilities.maxImageCount;
  }

  // Create swapchain
  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = _surface;
  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  // For simplicity, assume graphics and present are same queue family
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 0;
  createInfo.pQueueFamilyIndices = nullptr;

  createInfo.preTransform = capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = _oldSwapchain;

  if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create swapchain\n";
    return false;
  }

  // Get swapchain images
  vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
  _images.resize(imageCount);
  vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _images.data());

  // Store format and extent
  _imageFormat = surfaceFormat.format;
  _extent = extent;

  std::cout << "Swapchain created: " << _extent.width << "x" << _extent.height
            << " (" << imageCount << " images)\n";

  return true;
}

bool VulkanSwapchain::createImageViews() {
  _imageViews.resize(_images.size());

  for (size_t i = 0; i < _images.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = _images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = _imageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(_device, &createInfo, nullptr, &_imageViews[i]) !=
        VK_SUCCESS) {
      std::cerr << "Failed to create image view " << i << "\n";
      return false;
    }
  }

  return true;
}

bool VulkanSwapchain::createFramebuffers(VkRenderPass renderPass) {
  _framebuffers.resize(_imageViews.size());

  for (size_t i = 0; i < _imageViews.size(); i++) {
    VkImageView attachments[] = {_imageViews[i]};

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = 1;
    createInfo.pAttachments = attachments;
    createInfo.width = _extent.width;
    createInfo.height = _extent.height;
    createInfo.layers = 1;

    if (vkCreateFramebuffer(_device, &createInfo, nullptr, &_framebuffers[i]) !=
        VK_SUCCESS) {
      std::cerr << "Failed to create framebuffer " << i << "\n";
      return false;
    }
  }

  return true;
}

VkSurfaceFormatKHR VulkanSwapchain::chooseFormat(
    const std::vector<VkSurfaceFormatKHR> &available) {
  // Prefer SRGB if available
  for (const auto &format : available) {
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return available[0];
}

VkPresentModeKHR VulkanSwapchain::choosePresentMode(
    const std::vector<VkPresentModeKHR> &available) {
  // Prefer mailbox (triple buffering) if available
  for (const auto &mode : available) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  // FIFO is always available
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VulkanSwapchain::chooseExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  // Query actual framebuffer size
  int width, height;
  glfwGetFramebufferSize(_window, &width, &height);

  VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                             static_cast<uint32_t>(height)};

  actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);
  actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);

  return actualExtent;
}