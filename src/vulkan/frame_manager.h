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
#include "texture.h"
#include "depth.h"
#include "mesh.h"
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include "light.h"
#include "transform.h"

class VulkanFrameManager {
public:
VulkanFrameManager(
    Window &window, VulkanContext& context, VulkanSwapchain& swapchain,
    VulkanRenderer& renderer, VulkanCommands& commands, VulkanSync& sync,
    VulkanBuffer& buffer, Descriptor& descriptor, CameraState& camera,
    TransformPool& transforms,
    Depth& depth,
    MeshPool& meshes, TexturePool& textures, MaterialPool& materials, SunLight &light
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
    CameraState &m_camera;
    TransformPool &m_transforms;
    Depth &m_depth;
    MeshPool &m_meshes;
    TexturePool &m_textures;
    MaterialPool &m_materials;
    SunLight &m_light;
    VkExtent2D m_last_extent{};

    std::vector<VkFramebuffer> m_framebuffers;
    uint32_t m_current_frame = 0;

    void create_framebuffers();
    void recreate_swapchain();
    void cleanup_framebuffers();

    void ui_pass();
    void world_pass();
};
