#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>

namespace vulkan {

class VulkanCore {
public:
    VulkanCore() = default;
    ~VulkanCore();

    // Initialize with an already-created GLFW window
    // returns false on failure
    bool initialize(GLFWwindow* window);

    // Cleanup resources
    void cleanup();

    // Simple frame loop helper:
    // recordFunc is called inside an active renderpass with (cmdBuffer, imageIndex)
    bool drawFrame(const std::function<void(VkCommandBuffer, uint32_t)>& recordFunc);

    // Accessors for renderers
    auto device() const -> VkDevice { return device_; }
    auto physicalDevice() const -> VkPhysicalDevice { return physicalDevice_; }
    auto extent() const -> VkExtent2D { return extent_; }
    auto renderPass() const -> VkRenderPass { return renderPass_; }
    auto descriptorPool() const -> VkDescriptorPool { return descriptorPool_; }
    auto commandPool() const -> VkCommandPool { return commandPool_; }
    auto graphicsQueue() const -> VkQueue { return graphicsQueue_; }
    auto swapchainImageFormat() const -> VkFormat { return swapchainImageFormat_; }

private:
    // init steps
    bool createInstance();
    bool createSurface(GLFWwindow* window);
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
    auto chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avail) -> VkSurfaceFormatKHR;
    auto choosePresentMode(const std::vector<VkPresentModeKHR>& avail) -> VkPresentModeKHR;
    auto chooseExtent(const VkSurfaceCapabilitiesKHR& caps) -> VkExtent2D;

    auto checkValidationLayerSupport() const -> bool;
    auto setupDebugMessenger() -> void;
    auto destroyDebugMessenger() -> void;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT       messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT              messageType,
        const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData,
        void*                                        pUserData);


private:
#ifdef NDEBUG
    const bool enableValidationLayers_ = false;
#else
    const bool enableValidationLayers_ = true;
#endif

    VkInstance      instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice        device_ = VK_NULL_HANDLE;
    VkQueue         graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue         presentQueue_ = VK_NULL_HANDLE;
    VkSurfaceKHR    surface_ = VK_NULL_HANDLE;

    uint32_t graphicsFamily_ = UINT32_MAX;  // store graphics queue family index
    uint32_t presentFamily_ = UINT32_MAX;   // (optional, for clarity)

    VkSwapchainKHR       swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    VkFormat        swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D      extent_{ 800, 600 };

    VkRenderPass    renderPass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;

    VkCommandPool   commandPool_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;

    // sync
    std::vector<VkSemaphore> imageAvailable_;
    std::vector<VkSemaphore> renderFinished_;
    std::vector<VkFence> inFlightFences_;
    size_t currentFrame_ = 0;
    const size_t MAX_FRAMES_IN_FLIGHT = 2;

    VkDescriptorPool   descriptorPool_ = VK_NULL_HANDLE;
};
}