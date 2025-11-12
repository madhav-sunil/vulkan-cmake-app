#pragma once
#include <vulkan/vulkan.h>
#include "VulkanCore.hpp"

class VkApp {
public:
    VkApp();
    ~VkApp();

    bool initialize();
    void run();
    void cleanup();
    auto getWindow() const -> GLFWwindow* { return window; }
    auto getVulkanInstance()  ->  vulkan::VulkanCore& { return vulkanCore; }

private:
    // Vulkan-related members
    vulkan::VulkanCore vulkanCore;
    GLFWwindow* window = nullptr;

};