// text.cpp
#include "text.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "buffer.h"

Text::Text() = default;
Text::~Text()
{
  free(m_fontBuffer); 
}

// Wrapper around stbtt_GetCodepointSDF for a single glyph.
static unsigned char* generate_glyph_sdf(
    stbtt_fontinfo* info,
    int codepoint,
    float scale,
    int padding,
    unsigned char onedge_value,
    float pixel_dist_scale,
    int* out_w, int* out_h, int* out_xoff, int* out_yoff)
{
  return stbtt_GetCodepointSDF(
      info, scale, codepoint, padding, onedge_value, pixel_dist_scale,
      out_w, out_h, out_xoff, out_yoff);
}

void Text::loadFont(const std::string& fontPath, float pixelHeight)
{
  FILE* font_file = fopen(fontPath.c_str(), "rb");
  if (!font_file)
  {
    throw std::runtime_error("failed to open font file: " + fontPath);
  }

  fseek(font_file, 0, SEEK_END);
  long fileSize = ftell(font_file);
  fseek(font_file, 0, SEEK_SET);

  m_fontBuffer = static_cast<unsigned char*>(malloc(fileSize));
  fread(m_fontBuffer, 1, fileSize, font_file);
  fclose(font_file);

  stbtt_fontinfo info;
  if (!stbtt_InitFont(&info, m_fontBuffer, 0))
  {
    throw std::runtime_error("stbtt_InitFont failed for: " + fontPath);
  }

  m_pixelHeight = pixelHeight;
  float scale = stbtt_ScaleForPixelHeight(&info, pixelHeight);

  // --- simple row packer ---
  const int atlasW = 512;
  int penX = 0, penY = 0, rowH = 0;
  m_atlasWidth = atlasW;

  // First pass: generate all SDFs, figure out atlas height as we go.
  std::vector<std::pair<int, unsigned char*>> sdfs; // codepoint, buffer
  for (int codepoint = 32; codepoint <= 126; ++codepoint)
  {
    int ax, lsb;
    stbtt_GetCodepointHMetrics(&info, codepoint, &ax, &lsb);

    int w = 0, h = 0, xoff = 0, yoff = 0;
    unsigned char* sdf = generate_glyph_sdf(
        &info, codepoint, scale, 5, 180, 180.0f / 5.0f, &w, &h, &xoff, &yoff);

    GlyphInfo g{};
    g.width = w;
    g.height = h;
    g.xoff = xoff;
    g.yoff = yoff;
    g.advance = ax;

    if (sdf)
    {
      if (penX + w > atlasW) { penX = 0; penY += rowH; rowH = 0; }
      g.atlas_x = penX;
      g.atlas_y = penY;
      penX += w;
      rowH = std::max(rowH, h);
      sdfs.emplace_back(codepoint, sdf);
    }
    else
    {
      g.atlas_x = g.atlas_y = 0; // e.g. space — nothing to draw
    }

    m_glyphs[codepoint] = g;
  }

  m_atlasHeight = penY + rowH;
  m_atlasPixels.assign(static_cast<size_t>(m_atlasWidth) * m_atlasHeight, 0);

  // Second pass: blit each SDF into the atlas buffer at its assigned position.
  for (auto& [codepoint, sdf] : sdfs)
  {
    const GlyphInfo& g = m_glyphs[codepoint];
    for (int row = 0; row < g.height; ++row)
    {
      unsigned char* dst = &m_atlasPixels[(g.atlas_y + row) * m_atlasWidth + g.atlas_x];
      unsigned char* src = &sdf[row * g.width];
      memcpy(dst, src, g.width);
    }
    stbtt_FreeSDF(sdf, nullptr);
  }

  stbi_write_png("atlas_debug.png", m_atlasWidth, m_atlasHeight, 1,
               m_atlasPixels.data(), m_atlasWidth);

  // m_atlasPixels is now ready to upload to a GPU texture (or write to PNG for debugging).
}

void Text::text(float x, float y, const std::string& str, uint32_t size)
{
  // No font loading here — just walk the string, look up glyphs, emit quads.
  float penX = x;
  for (size_t i = 0; i < str.size(); ++i)
  {
    auto it = m_glyphs.find(str[i]);
    if (it == m_glyphs.end()) continue;

    const GlyphInfo& g = it->second;
    // TODO: push a quad here using g.atlas_x/atlas_y/width/height for UVs
    // and penX + g.xoff, y + g.yoff for position, scaled by (size / m_pixelHeight).

    penX += g.advance * (size / m_pixelHeight);


  }
}
