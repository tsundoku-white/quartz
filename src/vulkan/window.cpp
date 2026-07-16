#include "window.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <glm/ext/vector_float2.hpp>
#include <stdexcept>
#include "../core/core.h"

bool Window::is_key_pressed(Key key, PRESSED_TYPE pt)
{
  bool is_down = glfwGetKey(m_handle, static_cast<int>(key)) == GLFW_PRESS;

  if (pt == LOOP)
  {
    return is_down;
  }

  bool &was_pressed = m_key_was_pressed[static_cast<int>(key)];
  bool triggered = is_down && !was_pressed;
  was_pressed = is_down;
  return triggered;
}

glm::vec2 Window::get_mouse_loc()
{
  double xpos, ypos;
  glfwGetCursorPos(m_handle, &xpos, &ypos);
  return glm::vec2(xpos, ypos);
}

glm::vec2 Window::get_mouse_delta()
{
  double xpos, ypos;
  glfwGetCursorPos(m_handle, &xpos, &ypos);
  glm::vec2 current(xpos, ypos);

  if (m_first_mouse)
  {
    m_last_mouse_pos = current;
    m_first_mouse = false;
    return glm::vec2(0.0f, 0.0f);
  }

  glm::vec2 delta = current - m_last_mouse_pos;
  m_last_mouse_pos = current;
  return delta;
}

Window::Window()
{
  if (!glfwInit())
  {
    glfwTerminate();
    throw std::runtime_error(RED "Failed to load GLFW\n" RESET);
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_handle = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
  if (!m_handle)
  {
    glfwTerminate();
    throw std::runtime_error(RED "Failed to create window\n" RESET);
  }

  glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  std::print(GREEN "Pass:  " RESET "Create window\n");
}

Window::~Window()
{
  if (m_handle)
  {
    glfwDestroyWindow(m_handle);
  }
  glfwTerminate();
}
