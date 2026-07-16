#pragma once

#include "../core/core.h"
#include "src/vulkan/descriptor.h"
#include <vulkan/vulkan_core.h>

class Depth;

class VulkanRenderer {
  public: 
    VulkanRenderer(VulkanContext &context, VulkanSwapchain &swapchain, Descriptor &descriptor, Depth &depth);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer) = delete;

    [[nodiscard]] VkRenderPass      get_render_pass()     const { return m_render_pass;     }
    [[nodiscard]] VkPipeline        get_pipeline()        const { return m_pipeline;        }
    [[nodiscard]] VkPipelineLayout  get_pipeline_layout() const { return m_pipeline_layout; }

    void cleanup();

  private:
    VkPipeline        m_pipeline        = VK_NULL_HANDLE;
    VkPipelineLayout  m_pipeline_layout = VK_NULL_HANDLE;
    VkRenderPass      m_render_pass     = VK_NULL_HANDLE;

    std::string m_frag_shader_path = "shaders/g_frag.spv";
    std::string m_vert_shader_path = "shaders/g_vert.spv";

    VulkanContext &m_context;
    VulkanSwapchain &m_swapchain;
    Descriptor &m_descriptor;
    Depth      &m_depth;

    void create_shader();
    void create_graphics_pipeline();
    VkShaderModule create_shader_module(const std::vector<char>& code);
    void create_shader_stages(VkPipelineShaderStageCreateInfo stages[2]);
    void create_fixed_functions();
    void create_render_pass();
    VkPipelineShaderStageCreateInfo create_shader_stage(
        VkShaderModule module, VkShaderStageFlagBits stage);

};
