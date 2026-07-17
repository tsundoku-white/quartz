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
#include <array>
#include <memory>
#include <glm/ext/vector_float2.hpp>
#include <print>
#include <vector>

int main() {
  try {
    Window              window;
    VulkanContext       context     (window);
    VulkanSwapchain     swapchain   (context, window);
    VulkanSync          sync        (context, swapchain);
    VulkanBuffer        buffer      (context);
    Descriptor          descriptor  (context, swapchain, sync, buffer);
    CameraState         camera      = camera_system::create(swapchain, descriptor);
    
    Depth               depth       (context, swapchain);
    VulkanRenderer      renderer    (context, swapchain, descriptor, depth);
    VulkanCommands      commands    (context);

    SunLight            light       (descriptor);

    TexturePool  textures;
    MaterialPool materials;
    MeshPool     meshes;

    std::vector<MeshHandle> mesh_handles;

    for (int i = 0; i < 5; i++) {
        MeshHandle h = mesh_system::create(meshes);
        mesh_system::load(
            context, swapchain, descriptor, buffer,
            meshes, textures, materials, h
        );
        mesh_handles.push_back(h);
    }

    mesh_system::set_position(meshes, mesh_handles[0], {0.0f, 0.0f, 0.0f});
    mesh_system::set_position(meshes, mesh_handles[1], {3.0f, 0.0f, 0.0f});
    mesh_system::set_position(meshes, mesh_handles[2], {6.0f, 0.0f, 0.0f});
    mesh_system::set_position(meshes, mesh_handles[3], {9.0f, 0.0f, 0.0f});
    mesh_system::set_position(meshes, mesh_handles[4], {12.0f, 0.0f, 0.0f});

    VulkanFrameManager  frameManager(window, context, swapchain, renderer,
        commands, sync, buffer, descriptor, camera, depth, meshes, textures, materials, light);

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point lastFrameTime = Clock::now();
    constexpr float kMaxDeltaTime = 0.1f; 

    while (!window.should_close()) {
      Clock::time_point currentFrameTime = Clock::now();
      float delta_time = std::chrono::duration<float, std::chrono::seconds::period>(
          currentFrameTime - lastFrameTime).count();
      delta_time = std::min(delta_time, kMaxDeltaTime);
      lastFrameTime = currentFrameTime;

      window.poll_events();
 

      if (window.is_key_pressed(KEY_F1)) { 
        window.set_should_close(true); 
      }


      if (window.is_key_pressed(KEY_W)) {
        camera_system::move(camera, camera_system::get_forward_vector(camera), delta_time);
      }
      if (window.is_key_pressed(KEY_S)) {
        camera_system::move(camera, -camera_system::get_forward_vector(camera), delta_time);
      }


      if (window.is_key_pressed(KEY_A)) {
        camera_system::move(camera, -camera_system::get_right_vector(camera), delta_time);
      }
      if (window.is_key_pressed(KEY_D)) {
        camera_system::move(camera, camera_system::get_right_vector(camera), delta_time);
      }


      if (window.is_key_pressed(KEY_SPACE)) {
        camera_system::move(camera, glm::vec3(0, 1, 0), delta_time);
      }
      if (window.is_key_pressed(KEY_LEFT_SHIFT)) {
        camera_system::move(camera, glm::vec3(0, -1, 0), delta_time);
      }


      if (window.is_key_pressed(KEY_F2, ONCE)) {
        float fps = 1 / delta_time;
        std::print("FPS: {}\n", fps);
      }

      if (window.is_key_pressed(KEY_F3, ONCE)) {
        glm::vec3 forward = camera_system::get_forward_vector(camera);
        std::print("Camera position: ({:.2f}, {:.2f}, {:.2f})\n", 
            camera.position.x, camera.position.y, camera.position.z);
        std::print("Camera forward: ({:.2f}, {:.2f}, {:.2f})\n", 
            forward.x, forward.y, forward.z);
      }

      if (window.is_key_pressed(KEY_F4, ONCE)) {
        glm::vec2 loc = window.get_mouse_delta();
        std::print("mouse: {}, {}\n", loc.x, loc.y);
      }


      glm::vec2 md = window.get_mouse_delta();
      camera_system::rot(camera, -md.y * 0.2f, -md.x * 0.2f, 0.0f);

      frameManager.draw_frame();
    }
    context.wait_idle();
    mesh_system::destroy_all(context, meshes);
    texture_system::destroy_all(context, textures);
  } 
  catch (const std::exception& e) {
    std::print(RED "Error: " RESET "{}\n", e.what());
    return -1;
  }
  return 0;
}
