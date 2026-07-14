#pragma once
#include "../core/core.h"
#include <set>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class VulkanSwapchain {
  public: 
    VulkanSwapchain(VulkanContext &context, Window &window);

    [[nodiscard]] uint32_t get_image_count() const { return static_cast<uint32_t>(m_images.size()); }
    [[nodiscard]] VkExtent2D    get_extent()        const { return m_extent; }
    [[nodiscard]] std::vector<VkImageView>      get_image_views()  const { return m_image_views; }
    [[nodiscard]] VkFormat      get_image_format()  const { return m_image_format; }
    [[nodiscard]] VkSwapchainKHR get_swapchain() const { return m_swapchain; }
    [[nodiscard]] std::vector<VkImage> get_image() const { return m_images; }


    ~VulkanSwapchain();
    void cleanup();
    void create();

  private:
    VulkanContext &m_context;
    Window &m_window;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_image_format;
    VkExtent2D m_extent;
    
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_image_views;

    void create_swapchain();
    void create_image_views();

    VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
    VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
};

bool check_device_extension_support(VkPhysicalDevice device);
bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface);
SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface);
