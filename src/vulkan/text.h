#pragma once

#include "../core/core.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct stbtt_fontinfo;
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
    Text();
    ~Text();

    Text(const Text&) = delete;
    Text& operator=(const Text&) = delete;

    
    void loadFont(const std::string& fontPath, float pixelHeight);
    void text(float x, float y, const std::string& str, uint32_t size);

  private:
    unsigned char* m_fontBuffer = nullptr;
    std::unordered_map<int, GlyphInfo> m_glyphs;
    std::vector<unsigned char> m_atlasPixels;
    int m_atlasWidth = 0;
    int m_atlasHeight = 0;
    float m_pixelHeight = 0.0f;
};
