#include "frame_manager.h"
#include "buffer.h"
#include "src/vulkan/descriptor.h"
#include "src/vulkan/light.h"
#include "src/vulkan/mesh.h"
#include <stdexcept>
#include <vulkan/vulkan.h>
#include <glm/gtc/matrix_transform.hpp>

VulkanFrameManager::VulkanFrameManager(
    Window &window, VulkanContext& context, VulkanSwapchain& swapchain,
    VulkanRenderer& renderer, VulkanCommands& commands, VulkanSync& sync,
    VulkanBuffer& buffer, Descriptor& descriptor, Camera& camera, Material& quad_material,
    Depth& depth, Mesh &mesh, SunLight &light
    )
    : m_window(window), m_context(context), m_swapchain(swapchain)
    , m_renderer(renderer), m_commands(commands), m_sync(sync)
    , m_buffer(buffer), m_descriptor(descriptor)
    , m_camera(camera), m_quad_material(quad_material), m_depth(depth), m_mesh(mesh)
    , m_light(light)
{
    create_framebuffers();
    m_commands.create_command_buffers(swapchain.get_image_count());
}

VulkanFrameManager::~VulkanFrameManager()
{
    cleanup();
}

void VulkanFrameManager::create_framebuffers()
{
    cleanup_framebuffers();
    
    auto imageViews = m_swapchain.get_image_views();
    m_framebuffers.resize(imageViews.size());
    
    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = { imageViews[i], m_depth.get_image_view() };
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderer.get_render_pass();
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchain.get_extent().width;
        framebufferInfo.height = m_swapchain.get_extent().height;
        framebufferInfo.layers = 1;
        
        if (vkCreateFramebuffer(m_context.get_device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }
}

void VulkanFrameManager::draw_frame()
{
    auto [w, h] = m_window.get_framebuffer_size();

    if ((static_cast<uint32_t>(w) != m_last_extent.width ||
         static_cast<uint32_t>(h) != m_last_extent.height) &&
        w > 0 && h > 0)
    {
        recreate_swapchain();
        m_last_extent = m_swapchain.get_extent();
        return; 
    }

    m_sync.wait_for_fence(m_current_frame);
    m_sync.reset_fence(m_current_frame);

    uint32_t image_index;
    VkResult result = vkAcquireNextImageKHR(
        m_context.get_device(),
        m_swapchain.get_swapchain(),
        UINT64_MAX,
        m_sync.get_image_available(m_current_frame),
        VK_NULL_HANDLE,
        &image_index
    );

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image");
    }

    m_camera.update(m_current_frame, m_swapchain.get_extent());
    m_mesh.update(m_current_frame);
    m_light.update(m_current_frame);

    VkCommandBuffer command_buffer = m_commands.get_command_buffer(m_current_frame);
    vkResetCommandBuffer(command_buffer, 0);
    
    m_mesh.record(
        m_commands,
        m_current_frame,
        m_renderer.get_render_pass(),
        m_framebuffers[image_index],
        m_renderer.get_pipeline(),
        m_renderer.get_pipeline_layout(),
        m_quad_material.get_descriptor_set(m_current_frame),
        m_swapchain.get_extent()
    );

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { m_sync.get_image_available(m_current_frame) };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;


    VkSemaphore signal_semaphores[] = { m_sync.get_render_finished(image_index) };
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(m_context.get_graphics_queue(), 1, &submit_info,
          m_sync.get_in_flight(m_current_frame)) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = { m_swapchain.get_swapchain() };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;
    present_info.pImageIndices = &image_index;

    result = vkQueuePresentKHR(m_context.get_present_queue(), &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        recreate_swapchain();
    } else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to present swapchain image");
    }

    m_current_frame = (m_current_frame + 1) % VulkanSync::get_frame_count();
}

void VulkanFrameManager::recreate_swapchain()
{
    m_context.wait_idle();
    cleanup_framebuffers();  
    m_swapchain.cleanup();
    m_swapchain.create();
    m_depth.cleanup();
    m_depth.create_depth_resources(m_swapchain.get_extent());
    create_framebuffers(); 
    std::print("Recreate swapchain\n");
}

void VulkanFrameManager::cleanup_framebuffers()
{
    for (auto framebuffer : m_framebuffers) {
        if (framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(m_context.get_device(), framebuffer, nullptr);
        }
    }
    m_framebuffers.clear();
}

void VulkanFrameManager::cleanup()
{
    cleanup_framebuffers();
}
