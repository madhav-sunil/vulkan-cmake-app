#pragma once
#include <vector>
#include <vulkan/vulkan.h>

class TriangleRenderer {
public:
  TriangleRenderer(VkDevice device, VkRenderPass renderPass, VkExtent2D extent);
  ~TriangleRenderer();

  void recordCommands(VkCommandBuffer cmd);
  void resize(VkExtent2D newExtent);

private:
  VkDevice device_;
  VkRenderPass renderPass_;
  VkExtent2D extent_;

  VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
  VkPipeline graphicsPipeline_ = VK_NULL_HANDLE;

  void createPipeline();
  VkShaderModule loadShaderModule(const char *path);
};