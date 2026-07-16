#include "material.h"
#include "context.h"
#include <array>
#include <stdexcept>
#include "camera.h"
#include "descriptor.h"
#include "light.h"
#include "src/vulkan/mesh.h"

Material::Material(VulkanContext& context, Descriptor& descriptor, Texture& texture)
    : m_context(context)
{
    uint32_t frame_count = descriptor.get_frame_count();
    
    // Allocate one descriptor set PER FRAME
    std::vector<VkDescriptorSetLayout> layouts(frame_count, descriptor.get_layout());

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor.get_pool();
    alloc_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
    alloc_info.pSetLayouts = layouts.data();

    m_descriptor_sets.resize(layouts.size());
    if (vkAllocateDescriptorSets(m_context.get_device(), &alloc_info, m_descriptor_sets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // Update each descriptor set with the correct frame's buffer
    for (size_t i = 0; i < layouts.size(); i++)
    {
        VkBuffer uniform_buffer = descriptor.get_uniform_buffer(static_cast<uint32_t>(i));  // PER FRAME buffer!
        
        // Camera UBO (binding 0) - at offset 0
        VkDescriptorBufferInfo camera_buffer_info{};
        camera_buffer_info.buffer = uniform_buffer;
        camera_buffer_info.offset = 0;
        camera_buffer_info.range = sizeof(CAMERA_UBO);

        // Mesh UBO (binding 1) - after camera UBO
        VkDescriptorBufferInfo mesh_buffer_info{};
        mesh_buffer_info.buffer = uniform_buffer;
        mesh_buffer_info.offset = sizeof(CAMERA_UBO);
        mesh_buffer_info.range = sizeof(MESH_UBO);

        // Sun UBO (binding 3) - after camera + mesh UBO, matches
        // Descriptor::create_uniform_buffers() packing order
        VkDescriptorBufferInfo sun_buffer_info{};
        sun_buffer_info.buffer = uniform_buffer;
        sun_buffer_info.offset = sizeof(CAMERA_UBO) + sizeof(MESH_UBO);
        sun_buffer_info.range = sizeof(SUN_UBO);

        // Texture (binding 2) - shared across frames
        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView   = texture.get_image_view();
        image_info.sampler     = texture.get_sampler();

        // Camera UBO write (binding 0)
        VkWriteDescriptorSet camera_write{};
        camera_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        camera_write.dstSet = m_descriptor_sets[i];
        camera_write.dstBinding = 0;
        camera_write.descriptorCount = 1;
        camera_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        camera_write.pBufferInfo = &camera_buffer_info;

        // Mesh UBO write (binding 1)
        VkWriteDescriptorSet mesh_write{};
        mesh_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        mesh_write.dstSet = m_descriptor_sets[i];
        mesh_write.dstBinding = 1;
        mesh_write.descriptorCount = 1;
        mesh_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        mesh_write.pBufferInfo = &mesh_buffer_info;

        // Texture write (binding 2)
        VkWriteDescriptorSet texture_write{};
        texture_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        texture_write.dstSet = m_descriptor_sets[i];
        texture_write.dstBinding = 2;
        texture_write.descriptorCount = 1;
        texture_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texture_write.pImageInfo = &image_info;

        // Sun UBO write (binding 3)
        VkWriteDescriptorSet sun_write{};
        sun_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        sun_write.dstSet = m_descriptor_sets[i];
        sun_write.dstBinding = 3;
        sun_write.descriptorCount = 1;
        sun_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        sun_write.pBufferInfo = &sun_buffer_info;

        std::array<VkWriteDescriptorSet, 4> writes = {camera_write, mesh_write, texture_write, sun_write};
        vkUpdateDescriptorSets(m_context.get_device(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}

Material::~Material() {}
