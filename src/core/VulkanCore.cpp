#define GLFW_INCLUDE_VULKAN

#include "VulkanCore.hpp"
#include "vulkan/vulkan_core.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <set>

using namespace vulkan;

static const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
    ,
    "VK_KHR_portability_subset"
#endif
};

static const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  if (func)
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void
DestroyDebugUtilsMessengerEXT(VkInstance instance,
                              VkDebugUtilsMessengerEXT debugMessenger,
                              const VkAllocationCallbacks *pAllocator) {
  auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (func)
    func(instance, debugMessenger, pAllocator);
}

bool VulkanCore::checkValidationLayerSupport() const {
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> available(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, available.data());

  for (const char *layerName : validationLayers) {
    bool found = false;
    for (const auto &prop : available) {
      if (std::strcmp(prop.layerName, layerName) == 0) {
        found = true;
        break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

void VulkanCore::setupDebugMessenger() {
  if (!enableValidationLayers_)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = VulkanCore::debugCallback;
  createInfo.pUserData = nullptr;

  if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr,
                                   &_debugMessenger) != VK_SUCCESS) {
    std::cerr << "Failed to set up debug messenger\n";
  }
}

void VulkanCore::destroyDebugMessenger() {
  if (enableValidationLayers_ && _debugMessenger != VK_NULL_HANDLE) {
    DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    _debugMessenger = VK_NULL_HANDLE;
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanCore::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {

  (void)messageType;
  (void)pUserData;
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    std::cerr << "Validation ERROR: " << pCallbackData->pMessage << "\n";
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    std::cerr << "Validation WARNING: " << pCallbackData->pMessage << "\n";
  } else {
    std::cout << "Validation: " << pCallbackData->pMessage << "\n";
  }
  return VK_FALSE;
}

bool VulkanCore::initialize(GLFWwindow *window) {
  if (!createInstance())
    return false;
  setupDebugMessenger();
  if (!createSurface(window))
    return false;
  if (!pickPhysicalDevice())
    return false;
  if (!createLogicalDevice())
    return false;
  if (!createSwapchain())
    return false;
  if (!createImageViews())
    return false;
  if (!createRenderPass())
    return false;
  if (!createFramebuffers())
    return false;
  if (!createCommandPoolAndBuffers())
    return false;
  if (!createDescriptorPool())
    return false;
  if (!createSyncObjects())
    return false;
  return true;
}

VulkanCore::~VulkanCore() { cleanup(); }

void VulkanCore::cleanup() {
  vkDeviceWaitIdle(_device);

  for (auto f : _inFlightFences)
    vkDestroyFence(_device, f, nullptr);
  for (auto s : _imageAvailable)
    vkDestroySemaphore(_device, s, nullptr);
  for (auto s : _renderFinished)
    vkDestroySemaphore(_device, s, nullptr);

  if (_descriptorPool)
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

  if (_commandPool)
    vkDestroyCommandPool(_device, _commandPool, nullptr);

  for (auto fb : _framebuffers)
    vkDestroyFramebuffer(_device, fb, nullptr);
  if (_renderPass)
    vkDestroyRenderPass(_device, _renderPass, nullptr);

  for (auto iv : _swapchainImageViews)
    vkDestroyImageView(_device, iv, nullptr);
  if (_swapchain)
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

  if (_device)
    vkDestroyDevice(_device, nullptr);
  if (_surface)
    vkDestroySurfaceKHR(_instance, _surface, nullptr);

  destroyDebugMessenger();

  if (_instance)
    vkDestroyInstance(_instance, nullptr);
}

bool VulkanCore::createInstance() {

  if (enableValidationLayers_ && !checkValidationLayerSupport()) {
    std::cerr << "Validation layers requested, but not available!\n";
    return false;
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "vk-app";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "no-engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_2;

  uint32_t glfwExtCount = 0;
  const char **glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
  std::vector<const char *> extensions(glfwExt, glfwExt + glfwExtCount);

  if (enableValidationLayers_) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

#ifdef __APPLE__
  extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef __APPLE__
  createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

  if (enableValidationLayers_) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();
    // optionally provide debug messenger create info so loader can report early
    // issues
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = VulkanCore::debugCallback;
    createInfo.pNext = &debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
    std::cerr << "Failed to create instance\n";
    return false;
  }
  return true;
}

bool VulkanCore::createSurface(GLFWwindow *windowPtr) {
  if (glfwCreateWindowSurface(_instance, windowPtr, nullptr, &_surface) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create window surface\n";
    return false;
  }
  return true;
}

bool VulkanCore::pickPhysicalDevice() {
  uint32_t count = 0;
  vkEnumeratePhysicalDevices(_instance, &count, nullptr);
  if (count == 0) {
    std::cerr << "No GPUs with Vulkan support\n";
    return false;
  }
  std::vector<VkPhysicalDevice> devices(count);
  vkEnumeratePhysicalDevices(_instance, &count, devices.data());

  // pick first suitable
  for (auto dev : devices) {
    // check swapchain support + required extensions
    uint32_t extCount = 0;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, nullptr);
    std::vector<VkExtensionProperties> exts(extCount);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, exts.data());

    std::set<std::string> avail;
    for (auto &e : exts)
      avail.insert(e.extensionName);
    bool ok = true;
    for (auto req : deviceExtensions)
      if (!avail.count(req)) {
        ok = false;
        break;
      }

    if (!ok)
      continue;

    // surface capabilities check
    VkBool32 supported = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(dev, 0, _surface, &supported);
    // above is simplistic; just accept device if extensions present
    _physicalDevice = dev;
    break;
  }

  if (_physicalDevice == VK_NULL_HANDLE) {
    std::cerr << "Failed to find suitable GPU\n";
    return false;
  }
  return true;
}

bool VulkanCore::createLogicalDevice() {
  // find queue families
  uint32_t qCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &qCount, nullptr);
  std::vector<VkQueueFamilyProperties> qprops(qCount);
  vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &qCount,
                                           qprops.data());

  int graphicsFamily = -1;
  int presentFamily = -1;
  for (uint32_t i = 0; i < qCount; i++) {
    if (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      graphicsFamily = i;
    VkBool32 present = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface,
                                         &present);
    if (present)
      presentFamily = i;
  }
  if (graphicsFamily < 0 || presentFamily < 0) {
    std::cerr << "No suitable queue families\n";
    return false;
  }

  // store the graphics family index for later use
  _graphicsFamily = static_cast<uint32_t>(_graphicsFamily);
  _presentFamily = static_cast<uint32_t>(_presentFamily);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueFamilies = {graphicsFamily, presentFamily};
  float qPriority = 1.0f;
  for (int f : uniqueFamilies) {
    VkDeviceQueueCreateInfo qi{};
    qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qi.queueFamilyIndex = static_cast<uint32_t>(f);
    qi.queueCount = 1;
    qi.pQueuePriorities = &qPriority;
    queueCreateInfos.push_back(qi);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};
  VkDeviceCreateInfo dci{};
  dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  dci.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  dci.pQueueCreateInfos = queueCreateInfos.data();
  dci.pEnabledFeatures = &deviceFeatures;
  dci.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
  dci.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef __APPLE__
  dci.pNext = nullptr;
#endif
  dci.pNext = nullptr;

  if (vkCreateDevice(_physicalDevice, &dci, nullptr, &_device) != VK_SUCCESS) {
    std::cerr << "Failed to create logical device\n";
    return false;
  }

  vkGetDeviceQueue(_device, _graphicsFamily, 0, &_graphicsQueue);
  vkGetDeviceQueue(_device, _presentFamily, 0, &_presentQueue);
  return true;
}

VkSurfaceFormatKHR
VulkanCore::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &avail) {
  for (const auto &f : avail) {
    if (f.format == VK_FORMAT_B8G8R8A8_SRGB &&
        f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return f;
  }
  return avail[0];
}

VkPresentModeKHR
VulkanCore::choosePresentMode(const std::vector<VkPresentModeKHR> &avail) {
  for (const auto &p : avail)
    if (p == VK_PRESENT_MODE_MAILBOX_KHR)
      return p;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanCore::chooseExtent(const VkSurfaceCapabilitiesKHR &caps) {
  if (caps.currentExtent.width != UINT32_MAX)
    return caps.currentExtent;
  // fallback: use current window size (could be improved)
  _extent.width = std::clamp(_extent.width, caps.minImageExtent.width,
                             caps.maxImageExtent.width);
  _extent.height = std::clamp(_extent.height, caps.minImageExtent.height,
                              caps.maxImageExtent.height);
  return _extent;
}

bool VulkanCore::createSwapchain() {
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &caps);

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
                                       nullptr);
  if (formatCount == 0)
    return false;
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount,
                                       formats.data());

  uint32_t presentCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface,
                                            &presentCount, nullptr);
  std::vector<VkPresentModeKHR> presents(presentCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface,
                                            &presentCount, presents.data());

  VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
  VkPresentModeKHR presentMode = choosePresentMode(presents);
  VkExtent2D actualExtent = chooseExtent(caps);

  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    imageCount = caps.maxImageCount;

  VkSwapchainCreateInfoKHR sci{};
  sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  sci.surface = _surface;
  sci.minImageCount = imageCount;
  sci.imageFormat = surfaceFormat.format;
  sci.imageColorSpace = surfaceFormat.colorSpace;
  sci.imageExtent = actualExtent;
  sci.imageArrayLayers = 1;
  sci.imageUsage =
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  // assuming same queue for graphics/present for simplicity
  sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  sci.preTransform = caps.currentTransform;
  sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  sci.presentMode = presentMode;
  sci.clipped = VK_TRUE;
  sci.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(_device, &sci, nullptr, &_swapchain) != VK_SUCCESS) {
    std::cerr << "failed to create swapchain\n";
    return false;
  }

  vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
  _swapchainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount,
                          _swapchainImages.data());

  _swapchainImageFormat = surfaceFormat.format;
  _extent = actualExtent;
  return true;
}

bool VulkanCore::createImageViews() {
  _swapchainImageViews.resize(_swapchainImages.size());
  for (size_t i = 0; i < _swapchainImages.size(); ++i) {
    VkImageViewCreateInfo iv{};
    iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    iv.image = _swapchainImages[i];
    iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
    iv.format = _swapchainImageFormat;
    iv.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    iv.subresourceRange.baseMipLevel = 0;
    iv.subresourceRange.levelCount = 1;
    iv.subresourceRange.baseArrayLayer = 0;
    iv.subresourceRange.layerCount = 1;
    if (vkCreateImageView(_device, &iv, nullptr, &_swapchainImageViews[i]) !=
        VK_SUCCESS) {
      std::cerr << "failed to create image views\n";
      return false;
    }
  }
  return true;
}

bool VulkanCore::createRenderPass() {
  VkAttachmentDescription colorAtt{};
  colorAtt.format = _swapchainImageFormat;
  colorAtt.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAtt.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAtt.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAtt.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAtt.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAtt.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAtt.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;

  VkRenderPassCreateInfo rpci{};
  rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rpci.attachmentCount = 1;
  rpci.pAttachments = &colorAtt;
  rpci.subpassCount = 1;
  rpci.pSubpasses = &subpass;

  if (vkCreateRenderPass(_device, &rpci, nullptr, &_renderPass) != VK_SUCCESS) {
    std::cerr << "failed to create render pass\n";
    return false;
  }
  return true;
}

bool VulkanCore::createFramebuffers() {
  _framebuffers.resize(_swapchainImageViews.size());
  for (size_t i = 0; i < _swapchainImageViews.size(); ++i) {
    VkImageView attachments[] = {_swapchainImageViews[i]};
    VkFramebufferCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fci.renderPass = _renderPass;
    fci.attachmentCount = 1;
    fci.pAttachments = attachments;
    fci.width = _extent.width;
    fci.height = _extent.height;
    fci.layers = 1;
    if (vkCreateFramebuffer(_device, &fci, nullptr, &_framebuffers[i]) !=
        VK_SUCCESS) {
      std::cerr << "failed to create framebuffer\n";
      return false;
    }
  }
  return true;
}

bool VulkanCore::createCommandPoolAndBuffers() {

  VkCommandPoolCreateInfo cpci{};
  cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cpci.queueFamilyIndex = _graphicsFamily;
  cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(_device, &cpci, nullptr, &_commandPool) != VK_SUCCESS)
    return false;

  _commandBuffers.resize(_framebuffers.size());
  VkCommandBufferAllocateInfo cbai{};
  cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbai.commandPool = _commandPool;
  cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cbai.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());
  if (vkAllocateCommandBuffers(_device, &cbai, _commandBuffers.data()) !=
      VK_SUCCESS)
    return false;
  return true;
}

bool VulkanCore::createDescriptorPool() {
  VkDescriptorPoolSize poolSizes[1]{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(_framebuffers.size());

  VkDescriptorPoolCreateInfo dpci{};
  dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  dpci.poolSizeCount = 1;
  dpci.pPoolSizes = poolSizes;
  dpci.maxSets = static_cast<uint32_t>(_framebuffers.size());

  if (vkCreateDescriptorPool(_device, &dpci, nullptr, &_descriptorPool) !=
      VK_SUCCESS) {
    std::cerr << "failed to create descriptor pool\n";
    return false;
  }
  return true;
}

bool VulkanCore::createSyncObjects() {
  _imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
  _renderFinished.resize(MAX_FRAMES_IN_FLIGHT);
  _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo sci{};
  sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkFenceCreateInfo fci{};
  fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(_device, &sci, nullptr, &_imageAvailable[i]) !=
        VK_SUCCESS)
      return false;
    if (vkCreateSemaphore(_device, &sci, nullptr, &_renderFinished[i]) !=
        VK_SUCCESS)
      return false;
    if (vkCreateFence(_device, &fci, nullptr, &_inFlightFences[i]) !=
        VK_SUCCESS)
      return false;
  }
  return true;
}

bool VulkanCore::drawFrame(
    const std::function<void(VkCommandBuffer, uint32_t)> &recordFunc) {
  vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE,
                  UINT64_MAX);
  vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);

  uint32_t imageIndex;
  VkResult res = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX,
                                       _imageAvailable[_currentFrame],
                                       VK_NULL_HANDLE, &imageIndex);
  if (res == VK_ERROR_OUT_OF_DATE_KHR)
    return false;
  if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
    std::cerr << "failed to acquire image\n";
    return false;
  }

  // record command buffer: begin, begin renderpass, user callback, end
  // renderpass, end
  VkCommandBuffer cmd = _commandBuffers[imageIndex];
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo binfo{};
  binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &binfo);

  VkClearValue clearColor{{{0.0f, 0.0f, 0.0f, 1.0f}}};
  VkRenderPassBeginInfo rpbi{};
  rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpbi.renderPass = _renderPass;
  rpbi.framebuffer = _framebuffers[imageIndex];
  rpbi.renderArea.offset = {0, 0};
  rpbi.renderArea.extent = _extent;
  rpbi.clearValueCount = 1;
  rpbi.pClearValues = &clearColor;

  vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

  // user records draw commands here
  recordFunc(cmd, imageIndex);

  vkCmdEndRenderPass(cmd);
  vkEndCommandBuffer(cmd);

  VkSemaphore waitSem = _imageAvailable[_currentFrame];
  VkSemaphore signalSem = _renderFinished[_currentFrame];
  VkSubmitInfo submit{};
  submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  VkSemaphore waitSems[] = {waitSem};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = waitSems;
  submit.pWaitDstStageMask = waitStages;
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &cmd;
  submit.signalSemaphoreCount = 1;
  submit.pSignalSemaphores = &signalSem;

  if (vkQueueSubmit(_graphicsQueue, 1, &submit,
                    _inFlightFences[_currentFrame]) != VK_SUCCESS) {
    std::cerr << "failed to submit draw command buffer\n";
    return false;
  }

  VkPresentInfoKHR present{};
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.waitSemaphoreCount = 1;
  present.pWaitSemaphores = &signalSem;
  present.swapchainCount = 1;
  present.pSwapchains = &_swapchain;
  present.pImageIndices = &imageIndex;

  res = vkQueuePresentKHR(_presentQueue, &present);
  if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
    return false;
  if (res != VK_SUCCESS) {
    std::cerr << "failed to present swapchain image\n";
    return false;
  }

  _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  return true;
}