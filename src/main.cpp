#include "core/core.h"
#include "vulkan/window.h"
#include "vulkan/context.h"
#include "vulkan/swapchain.h"
#include "vulkan/renderer.h"
#include "vulkan/command.h"
#include "vulkan/sync.h"
#include "vulkan/frame_manager.h"
#include <cstdio>

int main() {
  try {
    Window              window;
    VulkanContext       context(window);
    VulkanSwapchain     swapchain(context, window);
    VulkanRenderer      renderer(context, swapchain);
    VulkanCommands      commands(context);
    VulkanSync          sync(context, swapchain.get_image_count());
    Buffer              buffer(context);

    VulkanFrameManager  frameManager(window, context, swapchain, renderer, commands, sync, buffer);

    while (!window.should_close()) {
      window.poll_events();
      frameManager.draw_frame();

      if (window.is_key_pressed(KEY_F1))
      {
        window.set_should_close(true);
      }
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
