// frame_manager.h
#pragma once
#include "window.h"
#include "context.h"
#include "swapchain.h"
#include "renderer.h"
#include "command.h"
#include "sync.h"
#include <vulkan/vulkan.h>
#include <vector>

class VulkanFrameManager {
public:
    VulkanFrameManager(
        Window &window,
        VulkanContext& context,
        VulkanSwapchain& swapchain,
        VulkanRenderer& renderer,
        VulkanCommands& commands,
        VulkanSync& sync
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
    VkExtent2D m_last_extent{};
    
    std::vector<VkFramebuffer> m_framebuffers;
    uint32_t m_current_frame = 0;
    
    void create_framebuffers();
    void recreate_swapchain();
    void cleanup_framebuffers();
};
