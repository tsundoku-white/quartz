#pragma once

#include "../core/core.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    bool is_complete() {
        return graphics_family.has_value() && present_family.has_value();
    }
};

class VulkanContext {
  public:
    VulkanContext(Window &window);
    void wait_idle();
    ~VulkanContext();

    [[nodiscard]] VkInstance        get_instance()         const { return m_instance;         }
    [[nodiscard]] VkDevice          get_device()           const { return m_device;           }
    [[nodiscard]] VkSurfaceKHR      get_surface()          const { return m_surface;          }
    [[nodiscard]] VkPhysicalDevice  get_physical_device()  const { return m_physical_device;  }
    [[nodiscard]] VkQueue           get_graphics_queue()   const { return m_graphics_queue;   }
    [[nodiscard]] VkQueue           get_present_queue()    const { return m_present_queue;    }

    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

  private:
    Window& m_window;
    VkInstance                m_instance            = VK_NULL_HANDLE;
    VkDevice                  m_device              = VK_NULL_HANDLE;
    VkSurfaceKHR              m_surface             = VK_NULL_HANDLE;
    VkPhysicalDevice          m_physical_device     = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT  m_debug_messenger     = VK_NULL_HANDLE;
    VkQueue                   m_graphics_queue      = VK_NULL_HANDLE;
    VkQueue                   m_present_queue       = VK_NULL_HANDLE;
    QueueFamilyIndices        m_queue_family_indices;

    void create_instance();
    void create_validation();
    void create_surface(Window &window);
    void create_logical_device();
    void create_physical_device();
    bool is_device_suitable(VkPhysicalDevice device);
};
