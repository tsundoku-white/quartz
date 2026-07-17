#pragma once

#include "../core/core.h"

struct GLFWwindow;

class Window {
  public:
    Window();
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window) = delete;

    [[nodiscard]] constexpr uint32_t get_width() const  { return m_width;  }
    [[nodiscard]] constexpr uint32_t get_height() const { return m_height; } 
    [[nodiscard]] bool should_close() const             { return glfwWindowShouldClose(m_handle); }
    [[nodiscard]] GLFWwindow* get_handle() const        { return m_handle; }

    [[nodiscard]] std::pair<int, int> get_framebuffer_size() const {
      int w, h;
      glfwGetFramebufferSize(m_handle, &w, &h);
      return {w, h};
    }

    inline void poll_events() { glfwPollEvents(); }

    void set_should_close(bool value) {
      glfwSetWindowShouldClose(m_handle, value ? GLFW_TRUE : GLFW_FALSE);
    }

    bool      is_key_pressed(Key key, PRESSED_TYPE pt = LOOP);
    glm::vec2 get_mouse_loc();
    glm::vec2 get_mouse_delta();
  private:
    GLFWwindow   *m_handle  = nullptr;
    uint32_t      m_width   = 600;
    uint32_t      m_height  = 600;
    std::string   m_title   = "untitled";
    std::unordered_map<int, bool> m_key_was_pressed;
    glm::vec2     m_last_mouse_pos = {0.0f, 0.0f};
    bool          m_first_mouse    = true;
};
