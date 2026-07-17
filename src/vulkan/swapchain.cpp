#include "swapchain.h"
#include "context.h"
#include "window.h"

const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool check_device_extension_support(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices;
    
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());
    
    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }
        
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            indices.present_family = i;
        }
        
        if (indices.is_complete()) {
            break;
        }
    }
    
    bool extensions_supported = check_device_extension_support(device);
    
    bool swap_chain_adequate = false;
    if (extensions_supported) {
        SwapChainSupportDetails swapchain_support = query_swapchain_support(device, surface);
        swap_chain_adequate = !swapchain_support.formats.empty() && 
                             !swapchain_support.present_modes.empty();
    }
    
    return indices.is_complete() && extensions_supported && swap_chain_adequate;
}

SwapChainSupportDetails query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }
    
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, 
                                                   details.present_modes.data());
    }
    
    return details;
}

void VulkanSwapchain::create_swapchain() {
    SwapChainSupportDetails swapchain_support = query_swapchain_support(
        m_context.get_physical_device(), 
        m_context.get_surface()
    );
    
    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);
    
    uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
    if (swapchain_support.capabilities.maxImageCount > 0 && 
        image_count > swapchain_support.capabilities.maxImageCount) {
        image_count = swapchain_support.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_context.get_surface();
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = m_context.find_queue_families(m_context.get_physical_device());
    uint32_t queue_family_indices[] = {
        indices.graphics_family.value(),
        indices.present_family.value()
    };
    
    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }
    
    create_info.preTransform = swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;
    
    if (vkCreateSwapchainKHR(m_context.get_device(), &create_info, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error(RED "Failed to create swapchain\n" RESET);
    }
    
    vkGetSwapchainImagesKHR(m_context.get_device(), m_swapchain, &image_count, nullptr);
    m_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_context.get_device(), m_swapchain, &image_count, m_images.data());
    
    m_image_format = surface_format.format;
    m_extent = extent;
    std::print(GREEN "Pass:  " RESET "Create swapchain\n");
}

void VulkanSwapchain::create_image_views() {
    m_image_views.resize(m_images.size());
    
    for (size_t i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = m_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = m_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(m_context.get_device(), &create_info, nullptr, &m_image_views[i]) != VK_SUCCESS) {
            throw std::runtime_error(RED "Failed to create image views\n" RESET);
        }
    }
    std::print(GREEN "Pass:  " RESET "Create image view\n");
}

VkSurfaceFormatKHR VulkanSwapchain::choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && 
            available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR VulkanSwapchain::choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& available_present_modes) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_window.get_handle(), &width, &height);
        
        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        
        actual_extent.width = std::clamp(actual_extent.width, 
                                        capabilities.minImageExtent.width, 
                                        capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, 
                                         capabilities.minImageExtent.height, 
                                         capabilities.maxImageExtent.height);
        
        return actual_extent;
    }
}

VulkanSwapchain::VulkanSwapchain(VulkanContext &context, Window &window) 
    : m_context(context), m_window(window) {
    create_swapchain();
    create_image_views();
}

void VulkanSwapchain::create()
{
    // Handle minimization: wait until the window has a real size again
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window.get_handle(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window.get_handle(), &width, &height);
        glfwWaitEvents();
    }

    create_swapchain();
    create_image_views();
}

void VulkanSwapchain::cleanup()
{
    for (auto image_view : m_image_views) {
        if (image_view != VK_NULL_HANDLE) {
            vkDestroyImageView(m_context.get_device(), image_view, nullptr);
        }
    }

    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_context.get_device(), m_swapchain, nullptr);
    }
}

VulkanSwapchain::~VulkanSwapchain()
{
  cleanup();
}
