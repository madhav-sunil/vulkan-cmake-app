#pragma once
#include <vulkan/vulkan.h>
#include "VulkanCore.hpp"
#include "TriangleRenderer.hpp"

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

    GLFWwindow* window = nullptr;
    vulkan::VulkanCore vulkanCore;
    std::unique_ptr<TriangleRenderer> triangleRenderer_;

};