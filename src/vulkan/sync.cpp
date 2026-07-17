#include "sync.h"
#include "context.h"
#include "swapchain.h"

VulkanSync::VulkanSync(VulkanContext& context, VulkanSwapchain &swapchain)
    : m_context(context)
    , m_swapchain(swapchain) 
{
    m_imageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
    m_renderFinished.resize(m_swapchain.get_image_count());
    m_inFlight.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_context.get_device(), &semaphoreInfo, nullptr, &m_imageAvailable[i]) != VK_SUCCESS || vkCreateFence(m_context.get_device(), &fenceInfo, nullptr, &m_inFlight[i]) != VK_SUCCESS)
        {
            throw std::runtime_error(RED RESET "Failed to create sync objects");
        }
    }

    for (size_t i = 0; i < m_swapchain.get_image_count(); i++) {
        if (vkCreateSemaphore(m_context.get_device(), &semaphoreInfo, nullptr, &m_renderFinished[i]) != VK_SUCCESS) {
            throw std::runtime_error(RED RESET"Failed to create sync objects");
        }
    }
}

VulkanSync::~VulkanSync()
{
    for (size_t i = 0; i < m_renderFinished.size(); i++) {
        if (m_renderFinished[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_context.get_device(), m_renderFinished[i], nullptr);
        }
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (m_imageAvailable[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_context.get_device(), m_imageAvailable[i], nullptr);
        }
        if (m_inFlight[i] != VK_NULL_HANDLE) {
            vkDestroyFence(m_context.get_device(), m_inFlight[i], nullptr);
        }
    }
}

void VulkanSync::wait_for_fence(uint32_t frameIndex)
{
    vkWaitForFences(m_context.get_device(), 1, &m_inFlight[frameIndex], VK_TRUE, UINT64_MAX);
}

void VulkanSync::reset_fence(uint32_t frameIndex)
{
    vkResetFences(m_context.get_device(), 1, &m_inFlight[frameIndex]);
}

void VulkanSync::wait_for_all_fences()
{
    vkDeviceWaitIdle(m_context.get_device());
}
