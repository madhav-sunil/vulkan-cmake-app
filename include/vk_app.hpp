#pragma once
#include <vulkan/vulkan.h>

class VkApp {
public:
    VkApp();
    ~VkApp();

    bool initialize();
    void run();
    void cleanup();

private:
    // Vulkan-related members
    VkInstance instance;
    VkDevice device;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    // Other necessary members for the application
};