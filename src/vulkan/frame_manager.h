#pragma once

#include "window.h"
#include "context.h"
#include "swapchain.h"
#include "renderer.h"
#include "command.h"
#include "sync.h"
#include "buffer.h"
#include "descriptor.h"
#include "camera.h"
#include "material.h"
#include "depth.h"
#include "mesh.h"
#include <vulkan/vulkan.h>
#include <vector>
#include "light.h"

class VulkanFrameManager {
public:
    VulkanFrameManager(
        Window &window,
        VulkanContext& context,
        VulkanSwapchain& swapchain,
        VulkanRenderer& renderer,
        VulkanCommands& commands,
        VulkanSync& sync,
        VulkanBuffer& buffer,
        Descriptor& descriptor,
        Camera& camera,
        Material& quad_material,
        Depth& depth,
        Mesh &mesh,
        SunLight &light
    );
    ~VulkanFrameManager();
    
    void draw_frame();
    void cleanup();
    
private:
    Window &m_window;
    VulkanContext& m_context;
    VulkanSwapchain& m_swapchain;
    VulkanRenderer& m_renderer;
    VulkanCommands& m_commands;
    VulkanSync& m_sync;
    VulkanBuffer &m_buffer;
    Descriptor &m_descriptor;
    Camera &m_camera;
    Material &m_quad_material;
    Depth &m_depth;
    Mesh &m_mesh;
    SunLight &m_light;
    VkExtent2D m_last_extent{};
    
    std::vector<VkFramebuffer> m_framebuffers;
    uint32_t m_current_frame = 0;
    
    void create_framebuffers();
    void recreate_swapchain();
    void cleanup_framebuffers();
};
