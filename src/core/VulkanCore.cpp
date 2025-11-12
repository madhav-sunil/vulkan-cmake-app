#include "VulkanCore.hpp"
#include <stdexcept>
#include <set>
#include <algorithm>
#include <iostream>

using namespace vulkan;

static const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
    , "VK_KHR_portability_subset"
#endif
};

static const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func) return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func) func(instance, debugMessenger, pAllocator);
}

bool VulkanCore::checkValidationLayerSupport() const {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> available(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, available.data());

    for (const char* layerName : validationLayers) {
        bool found = false;
        for (const auto& prop : available) {
            if (std::strcmp(prop.layerName, layerName) == 0) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

void VulkanCore::setupDebugMessenger() {
    if (!enableValidationLayers_) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = VulkanCore::debugCallback;
    createInfo.pUserData = nullptr;

    if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
        std::cerr << "Failed to set up debug messenger\n";
    }
}

void VulkanCore::destroyDebugMessenger() {
    if (enableValidationLayers_ && debugMessenger_ != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
        debugMessenger_ = VK_NULL_HANDLE;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanCore::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT              messageType,
    const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
    void*                                        pUserData) {

    (void)messageType; (void)pUserData;
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << "Validation ERROR: " << pCallbackData->pMessage << "\n";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Validation WARNING: " << pCallbackData->pMessage << "\n";
    } else {
        std::cout << "Validation: " << pCallbackData->pMessage << "\n";
    }
    return VK_FALSE;
}


bool VulkanCore::initialize(GLFWwindow* window) {
    if (!createInstance()) return false;
    setupDebugMessenger();
    if (!createSurface(window)) return false;
    if (!pickPhysicalDevice()) return false;
    if (!createLogicalDevice()) return false;
    if (!createSwapchain()) return false;
    if (!createImageViews()) return false;
    if (!createRenderPass()) return false;
    if (!createFramebuffers()) return false;
    if (!createCommandPoolAndBuffers()) return false;
    if (!createDescriptorPool()) return false;
    if (!createSyncObjects()) return false;
    return true;
}

VulkanCore::~VulkanCore() {
    cleanup();
}

void VulkanCore::cleanup() {
    vkDeviceWaitIdle(device_);

    for (auto f : inFlightFences_) vkDestroyFence(device_, f, nullptr);
    for (auto s : imageAvailable_) vkDestroySemaphore(device_, s, nullptr);
    for (auto s : renderFinished_) vkDestroySemaphore(device_, s, nullptr);

    if (descriptorPool_) vkDestroyDescriptorPool(device_, descriptorPool_, nullptr);

    if (commandPool_) vkDestroyCommandPool(device_, commandPool_, nullptr);

    for (auto fb : framebuffers_) vkDestroyFramebuffer(device_, fb, nullptr);
    if (renderPass_) vkDestroyRenderPass(device_, renderPass_, nullptr);

    for (auto iv : swapchainImageViews_) vkDestroyImageView(device_, iv, nullptr);
    if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr);

    if (device_) vkDestroyDevice(device_, nullptr);
    if (surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);

    destroyDebugMessenger();

    if (instance_) vkDestroyInstance(instance_, nullptr);
}

bool VulkanCore::createInstance() {

    if (enableValidationLayers_ && !checkValidationLayerSupport()) {
        std::cerr << "Validation layers requested, but not available!\n";
        return false;
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "vk-app";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pEngineName = "no-engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    uint32_t glfwExtCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    std::vector<const char*> extensions(glfwExt, glfwExt + glfwExtCount);

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
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        // optionally provide debug messenger create info so loader can report early issues
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
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

    if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
        std::cerr << "Failed to create instance\n";
        return false;
    }
    return true;
}

bool VulkanCore::createSurface(GLFWwindow* window) {
    if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS) {
        std::cerr << "Failed to create window surface\n";
        return false;
    }
    return true;
}

bool VulkanCore::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance_, &count, nullptr);
    if (count == 0) { std::cerr << "No GPUs with Vulkan support\n"; return false; }
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance_, &count, devices.data());

    // pick first suitable
    for (auto dev : devices) {
        // check swapchain support + required extensions
        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, nullptr);
        std::vector<VkExtensionProperties> exts(extCount);
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &extCount, exts.data());

        std::set<std::string> avail;
        for (auto &e : exts) avail.insert(e.extensionName);
        bool ok = true;
        for (auto req : deviceExtensions) if (!avail.count(req)) { ok = false; break; }

        if (!ok) continue;

        // surface capabilities check
        VkBool32 supported = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, 0, surface_, &supported);
        // above is simplistic; just accept device if extensions present
        physicalDevice_ = dev;
        break;
    }

    if (physicalDevice_ == VK_NULL_HANDLE) { std::cerr << "Failed to find suitable GPU\n"; return false; }
    return true;
}

bool VulkanCore::createLogicalDevice() {
    // find queue families
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qprops(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &qCount, qprops.data());

    int graphicsFamily = -1;
    int presentFamily = -1;
    for (uint32_t i=0;i<qCount;i++) {
        if (qprops[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) graphicsFamily = i;
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface_, &present);
        if (present) presentFamily = i;
    }
    if (graphicsFamily < 0 || presentFamily < 0) { std::cerr << "No suitable queue families\n"; return false; }

    // store the graphics family index for later use
    graphicsFamily_ = static_cast<uint32_t>(graphicsFamily);
    presentFamily_ = static_cast<uint32_t>(presentFamily);

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


    if (vkCreateDevice(physicalDevice_, &dci, nullptr, &device_) != VK_SUCCESS) {
        std::cerr << "Failed to create logical device\n";
        return false;
    }

    vkGetDeviceQueue(device_, graphicsFamily_, 0, &graphicsQueue_);
    vkGetDeviceQueue(device_, presentFamily_, 0, &presentQueue_);
    return true;
}

VkSurfaceFormatKHR VulkanCore::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avail) {
    for (const auto& f : avail) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return f;
    }
    return avail[0];
}
VkPresentModeKHR VulkanCore::choosePresentMode(const std::vector<VkPresentModeKHR>& avail) {
    for (const auto& p : avail) if (p == VK_PRESENT_MODE_MAILBOX_KHR) return p;
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D VulkanCore::chooseExtent(const VkSurfaceCapabilitiesKHR& caps) {
    if (caps.currentExtent.width != UINT32_MAX) return caps.currentExtent;
    // fallback: use current window size (could be improved)
    extent_.width = std::clamp(extent_.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    extent_.height = std::clamp(extent_.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    return extent_;
}

bool VulkanCore::createSwapchain() {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &caps);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);
    if (formatCount == 0) return false;
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, formats.data());

    uint32_t presentCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentCount, nullptr);
    std::vector<VkPresentModeKHR> presents(presentCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentCount, presents.data());

    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);
    VkPresentModeKHR presentMode = choosePresentMode(presents);
    VkExtent2D actualExtent = chooseExtent(caps);

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sci{};
    sci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sci.surface = surface_;
    sci.minImageCount = imageCount;
    sci.imageFormat = surfaceFormat.format;
    sci.imageColorSpace = surfaceFormat.colorSpace;
    sci.imageExtent = actualExtent;
    sci.imageArrayLayers = 1;
    sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    // assuming same queue for graphics/present for simplicity
    sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sci.preTransform = caps.currentTransform;
    sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sci.presentMode = presentMode;
    sci.clipped = VK_TRUE;
    sci.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device_, &sci, nullptr, &swapchain_) != VK_SUCCESS) {
        std::cerr << "failed to create swapchain\n"; return false;
    }

    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapchainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data());

    swapchainImageFormat_ = surfaceFormat.format;
    extent_ = actualExtent;
    return true;
}

bool VulkanCore::createImageViews() {
    swapchainImageViews_.resize(swapchainImages_.size());
    for (size_t i=0;i<swapchainImages_.size();++i) {
        VkImageViewCreateInfo iv{};
        iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv.image = swapchainImages_[i];
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format = swapchainImageFormat_;
        iv.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.baseMipLevel = 0;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.baseArrayLayer = 0;
        iv.subresourceRange.layerCount = 1;
        if (vkCreateImageView(device_, &iv, nullptr, &swapchainImageViews_[i]) != VK_SUCCESS) {
            std::cerr << "failed to create image views\n"; return false;
        }
    }
    return true;
}

bool VulkanCore::createRenderPass() {
    VkAttachmentDescription colorAtt{};
    colorAtt.format = swapchainImageFormat_;
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

    if (vkCreateRenderPass(device_, &rpci, nullptr, &renderPass_) != VK_SUCCESS) {
        std::cerr << "failed to create render pass\n"; return false;
    }
    return true;
}

bool VulkanCore::createFramebuffers() {
    framebuffers_.resize(swapchainImageViews_.size());
    for (size_t i=0;i<swapchainImageViews_.size();++i) {
        VkImageView attachments[] = { swapchainImageViews_[i] };
        VkFramebufferCreateInfo fci{};
        fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fci.renderPass = renderPass_;
        fci.attachmentCount = 1;
        fci.pAttachments = attachments;
        fci.width = extent_.width;
        fci.height = extent_.height;
        fci.layers = 1;
        if (vkCreateFramebuffer(device_, &fci, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
            std::cerr << "failed to create framebuffer\n"; return false;
        }
    }
    return true;
}

bool VulkanCore::createCommandPoolAndBuffers() {

    VkCommandPoolCreateInfo cpci{};
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.queueFamilyIndex = graphicsFamily_;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if (vkCreateCommandPool(device_, &cpci, nullptr, &commandPool_) != VK_SUCCESS) return false;

    commandBuffers_.resize(framebuffers_.size());
    VkCommandBufferAllocateInfo cbai{};
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool = commandPool_;
    cbai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());
    if (vkAllocateCommandBuffers(device_, &cbai, commandBuffers_.data()) != VK_SUCCESS) return false;
    return true;
}

bool VulkanCore::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[1]{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(framebuffers_.size());

    VkDescriptorPoolCreateInfo dpci{};
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = poolSizes;
    dpci.maxSets = static_cast<uint32_t>(framebuffers_.size());

    if (vkCreateDescriptorPool(device_, &dpci, nullptr, &descriptorPool_) != VK_SUCCESS) {
        std::cerr << "failed to create descriptor pool\n"; return false;
    }
    return true;
}

bool VulkanCore::createSyncObjects() {
    imageAvailable_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinished_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo sci{};
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fci{};
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i=0;i<MAX_FRAMES_IN_FLIGHT;i++) {
        if (vkCreateSemaphore(device_, &sci, nullptr, &imageAvailable_[i]) != VK_SUCCESS) return false;
        if (vkCreateSemaphore(device_, &sci, nullptr, &renderFinished_[i]) != VK_SUCCESS) return false;
        if (vkCreateFence(device_, &fci, nullptr, &inFlightFences_[i]) != VK_SUCCESS) return false;
    }
    return true;
}

bool VulkanCore::drawFrame(const std::function<void(VkCommandBuffer, uint32_t)>& recordFunc) {
    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, imageAvailable_[currentFrame_], VK_NULL_HANDLE, &imageIndex);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) return false;
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) { std::cerr << "failed to acquire image\n"; return false; }

    // record command buffer: begin, begin renderpass, user callback, end renderpass, end
    VkCommandBuffer cmd = commandBuffers_[imageIndex];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo binfo{};
    binfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd, &binfo);

    VkClearValue clearColor{ {{0.0f, 0.0f, 0.0f, 1.0f}} };
    VkRenderPassBeginInfo rpbi{};
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.renderPass = renderPass_;
    rpbi.framebuffer = framebuffers_[imageIndex];
    rpbi.renderArea.offset = {0,0};
    rpbi.renderArea.extent = extent_;
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = &clearColor;

    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    // user records draw commands here
    recordFunc(cmd, imageIndex);

    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    VkSemaphore waitSem = imageAvailable_[currentFrame_];
    VkSemaphore signalSem = renderFinished_[currentFrame_];

    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSems[] = { waitSem };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = waitSems;
    submit.pWaitDstStageMask = waitStages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &signalSem;

    if (vkQueueSubmit(graphicsQueue_, 1, &submit, inFlightFences_[currentFrame_]) != VK_SUCCESS) {
        std::cerr << "failed to submit draw command buffer\n"; return false;
    }

    VkPresentInfoKHR present{};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.waitSemaphoreCount = 1;
    present.pWaitSemaphores = &signalSem;
    present.swapchainCount = 1;
    present.pSwapchains = &swapchain_;
    present.pImageIndices = &imageIndex;

    res = vkQueuePresentKHR(presentQueue_, &present);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) return false;
    if (res != VK_SUCCESS) { std::cerr << "failed to present swapchain image\n"; return false; }

    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}