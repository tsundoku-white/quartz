#pragma once

#include "../core/core.h"

class VulkanSync {
public:
    VulkanSync(VulkanContext& context, VulkanSwapchain &swapchain);
    ~VulkanSync();

    VulkanSync(const VulkanSync&) = delete;
    VulkanSync& operator=(const VulkanSync&) = delete;

    [[nodiscard]] VkSemaphore get_image_available(uint32_t frame)       const { return m_imageAvailable[frame]; }
    [[nodiscard]] VkFence     get_in_flight      (uint32_t frame)       const { return m_inFlight[frame]; }
    [[nodiscard]] VkSemaphore get_render_finished(uint32_t imageIndex)  const { return m_renderFinished[imageIndex]; }

    void wait_for_fence(uint32_t frameIndex);
    void reset_fence(uint32_t frameIndex);
    void wait_for_all_fences();

    static constexpr uint32_t get_frame_count() { return MAX_FRAMES_IN_FLIGHT; }

private:
    VulkanContext& m_context;
    VulkanSwapchain &m_swapchain;
    
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
    
    std::vector<VkSemaphore> m_imageAvailable;
    std::vector<VkSemaphore> m_renderFinished;
    std::vector<VkFence>     m_inFlight;
};
