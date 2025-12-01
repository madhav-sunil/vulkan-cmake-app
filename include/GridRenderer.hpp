#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct GridPushConstants {
  glm::mat4 viewProj;
  glm::mat4 invViewProj;
  glm::vec3 cameraPos;
  float gridScale;
};

class GridRenderer {
public:
  GridRenderer(VkDevice device, VkRenderPass renderPass, VkExtent2D extent);
  ~GridRenderer();

  void recordCommands(VkCommandBuffer cmd, const GridPushConstants &constants);
  void resize(VkExtent2D newExtent);

private:
  VkDevice _device;
  VkRenderPass _renderPass;
  VkExtent2D _extent;

  VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
  VkPipeline _graphicsPipeline = VK_NULL_HANDLE;

  void createPipeline();
  VkShaderModule loadShaderModule(const char *path);
};