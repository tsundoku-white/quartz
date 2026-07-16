#pragma once

#include "../core/core.h"
#include "buffer.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct stbtt_fontinfo;
class VulkanContext;

struct GlyphInfo
{
  int width, height;
  int xoff, yoff;
  int atlas_x, atlas_y;
  int advance;
};

class Text
{
  public:
    Text(VulkanContext& context, VulkanBuffer& buffer, uint32_t frame_count);
    ~Text();

    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;

    void loadFont(const std::string& fontPath, float pixelHeight);
    void begin_frame();
    void text(float x, float y, const std::string& str, uint32_t size);
    void update(uint32_t current_frame);

    [[nodiscard]] VkBuffer get_vertex_buffer(uint32_t frame) const { return m_vertex_buffers[frame]; }
    [[nodiscard]] uint32_t get_vertex_count() const { return static_cast<uint32_t>(m_vertices.size()); }

    [[nodiscard]] VkImageView get_atlas_view()    const { return m_atlas_view; }
    [[nodiscard]] VkSampler   get_atlas_sampler() const { return m_atlas_sampler; }

  private:
    VulkanContext& m_context;
    VulkanBuffer&  m_buffer;

    unsigned char* m_fontBuffer = nullptr;
    std::unordered_map<int, GlyphInfo> m_glyphs;
    std::vector<unsigned char> m_atlasPixels;
    int   m_atlasWidth  = 0;
    int   m_atlasHeight = 0;
    float m_pixelHeight = 0.0f;

    std::vector<Vertex> m_vertices;
    static constexpr size_t kMaxVertices = 4096;

    std::vector<VkBuffer>       m_vertex_buffers;
    std::vector<VkDeviceMemory> m_vertex_buffers_memory;
    std::vector<void*>          m_mapped;

    VkImage        m_atlas_image        = VK_NULL_HANDLE;
    VkDeviceMemory m_atlas_image_memory = VK_NULL_HANDLE;
    VkImageView    m_atlas_view         = VK_NULL_HANDLE;
    VkSampler      m_atlas_sampler      = VK_NULL_HANDLE;

    void create_vertex_buffers(uint32_t frame_count);
    void create_atlas_texture();
};
