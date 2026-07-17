#include "texture.h"
#include "buffer.h"
#include "context.h"
#include "swapchain.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static VkCommandPool create_command_pool(VulkanContext& context)
{
    QueueFamilyIndices queue_family_indices = context.find_queue_families(context.get_physical_device());

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

    VkCommandPool command_pool;
    if (vkCreateCommandPool(context.get_device(), &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    return command_pool;
}

static VkCommandBuffer begin_single_time_commands(VulkanContext& context, VkCommandPool command_pool)
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(context.get_device(), &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

static void end_single_time_commands(VulkanContext& context, VkCommandPool command_pool, VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    VkQueue graphics_queue = context.get_graphics_queue();
    vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(context.get_device(), command_pool, 1, &command_buffer);
}

static void create_image(VulkanContext& context, VulkanBuffer& buffer,
    uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
    VkImage& image, VkDeviceMemory& image_memory)
{
    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(context.get_device(), &image_info, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(context.get_device(), image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = buffer.find_memory_type(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(context.get_device(), &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(context.get_device(), image, image_memory, 0);
}

static void transition_image_layout(VulkanContext& context, VkCommandPool command_pool,
    VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer = begin_single_time_commands(context, command_pool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    end_single_time_commands(context, command_pool, command_buffer);
}

static void copy_buffer_to_image(VulkanContext& context, VkCommandPool command_pool,
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer command_buffer = begin_single_time_commands(context, command_pool);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands(context, command_pool, command_buffer);
}

static VkImageView create_image_view(VulkanContext& context, VkImage image, VkFormat format)
{
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(context.get_device(), &view_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view");
    }
    return image_view;
}

static VkSampler create_texture_sampler(VulkanContext& context)
{
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(context.get_physical_device(), &properties);
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    VkSampler sampler;
    if (vkCreateSampler(context.get_device(), &sampler_info, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler");
    }
    return sampler;
}

TextureHandle texture_system::create(TexturePool& pool, VulkanContext& context, VulkanSwapchain& swapchain,
                                      VulkanBuffer& buffer, const std::string& path)
{
    TextureHandle handle{ pool.count };

    VkCommandPool command_pool = create_command_pool(context);

    stbi_set_flip_vertically_on_load(true);
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(path.c_str(), &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkDeviceSize image_size = static_cast<VkDeviceSize>(tex_width) * static_cast<VkDeviceSize>(tex_height) * 4;
    uint32_t width  = static_cast<uint32_t>(tex_width);
    uint32_t height = static_cast<uint32_t>(tex_height);

    VkBuffer       staging_buffer        = VK_NULL_HANDLE;
    VkDeviceMemory staging_buffer_memory = VK_NULL_HANDLE;

    buffer.create_buffer(image_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_buffer_memory);

    void* data;
    vkMapMemory(context.get_device(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(context.get_device(), staging_buffer_memory);
    stbi_image_free(pixels);

    VkImage        image        = VK_NULL_HANDLE;
    VkDeviceMemory image_memory = VK_NULL_HANDLE;

    create_image(context, buffer, width, height, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        image, image_memory);

    transition_image_layout(context, command_pool, image, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copy_buffer_to_image(context, command_pool, staging_buffer, image, width, height);

    transition_image_layout(context, command_pool, image, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(context.get_device(), staging_buffer, nullptr);
    vkFreeMemory(context.get_device(), staging_buffer_memory, nullptr);

    VkImageView image_view = create_image_view(context, image, VK_FORMAT_R8G8B8A8_SRGB);
    VkSampler   sampler    = create_texture_sampler(context);

    pool.staging_buffer.push_back(VK_NULL_HANDLE);
    pool.staging_buffer_memory.push_back(VK_NULL_HANDLE);
    pool.image.push_back(image);
    pool.image_memory.push_back(image_memory);
    pool.command_pool.push_back(command_pool);
    pool.image_view.push_back(image_view);
    pool.sampler.push_back(sampler);
    pool.width.push_back(width);
    pool.height.push_back(height);
    pool.path.push_back(path);

    pool.count++;

    std::print(GREEN "PASS:  " RESET "create texture\n");

    return handle;
}

void texture_system::destroy(VulkanContext& context, TexturePool& pool, TextureHandle handle)
{
    uint32_t i = handle.index;

    if (pool.sampler[i] != VK_NULL_HANDLE)
        vkDestroySampler(context.get_device(), pool.sampler[i], nullptr);
    if (pool.image_view[i] != VK_NULL_HANDLE)
        vkDestroyImageView(context.get_device(), pool.image_view[i], nullptr);
    if (pool.image[i] != VK_NULL_HANDLE)
        vkDestroyImage(context.get_device(), pool.image[i], nullptr);
    if (pool.image_memory[i] != VK_NULL_HANDLE)
        vkFreeMemory(context.get_device(), pool.image_memory[i], nullptr);
    if (pool.command_pool[i] != VK_NULL_HANDLE)
        vkDestroyCommandPool(context.get_device(), pool.command_pool[i], nullptr);

    pool.sampler[i]      = VK_NULL_HANDLE;
    pool.image_view[i]   = VK_NULL_HANDLE;
    pool.image[i]        = VK_NULL_HANDLE;
    pool.image_memory[i] = VK_NULL_HANDLE;
    pool.command_pool[i] = VK_NULL_HANDLE;
}

void texture_system::destroy_all(VulkanContext& context, TexturePool& pool)
{
    for (uint32_t i = 0; i < pool.count; i++) {
        destroy(context, pool, TextureHandle{ i });
    }
}
