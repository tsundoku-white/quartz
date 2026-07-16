# QUARTZ — Vulkan Rendering Library

> A lightweight Vulkan-based rendering library designed for simplicity and performance.

---

## Current Features

| Feature           | Description                                                                 |
|-------------------|-----------------------------------------------------------------------------|
| **Model Loading** | Load `.glb` models with normal support and optional texture mapping.       |
| **Texture Loading**| Supports `.png` and `.jpg` image formats.                                 |
| **Input Handling**| Mouse position, delta movement, and keyboard input.                        |
| **Lighting**      | Global directional sunlight.                                               |

---

## Upcoming Features

- [ ] Animations  
- [ ] Point & area lights  
- [ ] UI — images, text, buttons  
- [ ] Physics — colliders, raycasts, kinematics, rigid bodies  

---

## Dependencies

Make sure the following are installed on your system:

- [GLFW](https://www.glfw.org/)  
- [Vulkan SDK](https://vulkan.lunarg.com/)  
- C++26 compatible compiler (e.g., Clang)  
- [CMake](https://cmake.org/)  
- [Ninja](https://ninja-build.org/) or Make  

### External Libraries (in `/ext`)

- [glTF](https://github.com/sketchfab/gltf)  
- [GLM](https://github.com/g-truc/glm)  
- [stb](https://github.com/nothings/stb)  

---

## Platform Support

Currently supported on **Linux only**.  
Windows support is planned but not yet tested.

---

## Installation

[Linux Installation Guide](docs/linux_install.md)
An installation script for Windows is **coming soon**.  

---

## License
none use it how ever you want.
