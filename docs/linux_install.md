## Linux Install 

# Arch Linux
```bash 
sudo pacman -S \
    base-devel \
    cmake \
    ninja \
    clang \
    glfw \
    vulkan-devel \
    vulkan-icd-loader \
    vulkan-headers \
    shaderc
```

```bash 
    mkdir build && cd build
    cmake .. -G Ninja
    ninja
    sudo ninja Install
```
