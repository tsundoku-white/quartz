#include "core/core.h"
#include "src/vulkan/light.h"
#include "src/vulkan/transform.h"
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

    TransformPool transforms;

    CameraState         camera      = camera_system::create(swapchain, descriptor, transforms);

    Depth               depth       (context, swapchain);
    VulkanRenderer      renderer    (context, swapchain, descriptor, depth);
    VulkanCommands      commands    (context);

    SunLight            light       (descriptor);

    TexturePool  textures;
    MaterialPool materials;
    MeshPool     meshes;

    std::vector<MeshHandle> mesh_handles;
    const int gridSize = 10;
    const float spacing = 1.5f;
    const float half = (gridSize - 1) / 2.0f;

    mesh_handles.reserve(static_cast<size_t>(gridSize) * gridSize * gridSize);

    for (int x = 0; x < gridSize; x++) {
      const float posX = (x - half) * spacing;
      for (int y = 0; y < gridSize; y++) {
        const float posY = (y - half) * spacing;
        for (int z = 0; z < gridSize; z++) {
          const float posZ = (z - half) * spacing;

          MeshHandle h = mesh_system::create(meshes, transforms);
          mesh_system::load(
              context, swapchain, descriptor, buffer,
              meshes, textures, materials, h
              );
          mesh_handles.push_back(h);

          mesh_system::set_position(meshes, h, transforms, {posX, posY, posZ});
        }
      }
    }

    VulkanFrameManager  frameManager(window, context, swapchain, renderer,
        commands, sync, buffer, descriptor, camera, transforms, depth, meshes, textures, materials, light);

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
        camera_system::move(camera, transforms, camera_system::get_forward_vector(camera, transforms), delta_time);
      }
      if (window.is_key_pressed(KEY_S)) {
        camera_system::move(camera, transforms, -camera_system::get_forward_vector(camera, transforms), delta_time);
      }

      if (window.is_key_pressed(KEY_A)) {
        camera_system::move(camera, transforms, -camera_system::get_right_vector(camera, transforms), delta_time);
      }
      if (window.is_key_pressed(KEY_D)) {
        camera_system::move(camera, transforms, camera_system::get_right_vector(camera, transforms), delta_time);
      }

      if (window.is_key_pressed(KEY_SPACE)) {
        camera_system::move(camera, transforms, glm::vec3(0, 1, 0), delta_time);
      }
      if (window.is_key_pressed(KEY_LEFT_SHIFT)) {
        camera_system::move(camera, transforms, glm::vec3(0, -1, 0), delta_time);
      }

      if (window.is_key_pressed(KEY_F2, ONCE)) {
        float fps = 1 / delta_time;
        std::print("FPS: {}\n", fps);
      }

      if (window.is_key_pressed(KEY_F4, ONCE)) {
        glm::vec2 loc = window.get_mouse_delta();
        std::print("mouse: {}, {}\n", loc.x, loc.y);
      }

      glm::vec2 md = window.get_mouse_delta();
      camera_system::rot(camera, transforms, -md.y * 0.2f, -md.x * 0.2f, 0.0f);

      transform_system::update_all(transforms);

      frameManager.draw_frame();
    }
    context.wait_idle();

    mesh_system::destroy_all(context, meshes);
    texture_system::destroy_all(context, textures);
    transform_system::destroy_all(transforms);
  }
  catch (const std::exception& e) {
    std::print(RED "Error: " RESET "{}\n", e.what());
    return -1;
  }
  return 0;
}
