#include "core/core.h"
#include "src/vulkan/light.h"
#include "vulkan/window.h"
#include "vulkan/context.h"
#include "vulkan/swapchain.h"
#include "vulkan/renderer.h"
#include "vulkan/command.h"
#include "vulkan/sync.h"
#include "vulkan/frame_manager.h"
#include "vulkan/camera.h"
#include "vulkan/texture.h"
#include "vulkan/text.h"
#include "vulkan/descriptor.h"
#include "vulkan/material.h"
#include "vulkan/depth.h"
#include "vulkan/mesh.h"

#include <cstdio>
#include <chrono>
#include <algorithm>
#include <glm/ext/vector_float2.hpp>
#include <print>

// You might want to define these mouse variables
static float mouse_x = 0.0f;
static float mouse_y = 0.0f;
static bool first_mouse = true;

int main() {
  try {
    Window              window;
    VulkanContext       context     (window);
    VulkanSwapchain     swapchain   (context, window);
    VulkanSync          sync        (context, swapchain);
    VulkanBuffer        buffer      (context);
    Descriptor          descriptor  (context, swapchain, sync, buffer);
    Camera              camera      (swapchain, descriptor);
    Texture             texture     (context, swapchain, buffer);
    Depth               depth       (context, swapchain);
    VulkanRenderer      renderer    (context, swapchain, descriptor, depth);
    VulkanCommands      commands    (context);
    Material            material    (context, descriptor, texture);
    Mesh                mesh        (context, swapchain, descriptor, buffer);
    SunLight            light       (descriptor); 

    VulkanFrameManager  frameManager(window, context, swapchain, renderer,
        commands, sync, buffer, descriptor, camera, material, depth, mesh, light);

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastFrameTime = Clock::now();
    constexpr float kMaxDeltaTime = 0.1f; 

    bool is_printing = false;

    while (!window.should_close()) {
      Clock::time_point currentFrameTime = Clock::now();
      float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(
          currentFrameTime - lastFrameTime).count();
      delta_time = std::min(delta_time, kMaxDeltaTime);
      lastFrameTime = currentFrameTime;

      window.poll_events();
 
      // Exit
      if (window.is_key_pressed(KEY_F1)) { 
        window.set_should_close(true); 
      }

      // Movement - Forward/Backward
      if (window.is_key_pressed(KEY_W)) {
        camera.move(camera.get_forward_vector(), delta_time);
      }
      if (window.is_key_pressed(KEY_S)) {
        camera.move(-camera.get_forward_vector(), delta_time);
      }

      // Movement - Strafe Left/Right
      if (window.is_key_pressed(KEY_A)) {
        camera.move(-camera.get_right_vector(), delta_time);
      }
      if (window.is_key_pressed(KEY_D)) {
        camera.move(camera.get_right_vector(), delta_time);
      }

      // Movement - Up/Down
      if (window.is_key_pressed(KEY_SPACE)) {
        camera.move(glm::vec3(0, 1, 0), delta_time);
      }
      if (window.is_key_pressed(KEY_LEFT_SHIFT)) {
        camera.move(glm::vec3(0, -1, 0), delta_time);
      }

      if (window.is_key_pressed(KEY_F2, ONCE))
      {
        float fps = 1 / delta_time;
        std::print("FPS: {}\n", fps);
      }

      if (window.is_key_pressed(KEY_F4, ONCE)) 
      {
        glm::vec2 loc =  window.get_mouse_delta();
        std::print("mouse: {}, {}\n", loc.x, loc.y);
      }

      glm::vec2 md = window.get_mouse_delta();
      camera.rot(-md.y * 0.2f, -md.x * 0.2f, 0.0f);


      frameManager.draw_frame();
    }
    context.wait_idle();
  } 
  catch (const std::exception& e)
  {
    std::print(RED "Error: " RESET "{}\n", e.what());
    return -1;
  }
  return 0;
}
