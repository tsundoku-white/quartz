#include "window.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <stdexcept>
#include "../core/core.h"

bool Window::is_key_pressed(Key key)
{
    return glfwGetKey(m_handle, static_cast<int>(key)) == GLFW_PRESS;
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
