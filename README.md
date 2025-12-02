# Vulkan CMake Application

A modern Vulkan-based graphics application featuring a camera system, grid renderer, and modular architecture built with C++17 and CMake.

## Features

- **Vulkan Rendering Pipeline**: Modern Vulkan-based rendering with swapchain management
- **Camera System**: 
  - Free-fly camera (FPS-style movement)
  - Orbit camera (Blender-style, target-focused)
  - Flexible camera controller architecture
- **Grid Renderer**: Infinite grid with multi-scale visualization and axis indicators
- **Input System**: Configurable key bindings and mouse controls

## Project Structure

```
vulkan-cmake-app/
├── CMakeLists.txt          # Main build configuration
├── README.md               # This file
├── LICENSE                 # MIT License
├── .gitignore             # Git ignore rules
│
├── include/               # Public headers
│   ├── VkApp.hpp          # Main application class
│   ├── VulkanCore.hpp     # Core Vulkan initialization
│   ├── VulkanSwapchain.hpp # Swapchain management
│   ├── Camera.hpp         # Camera class
│   ├── CameraController.hpp # Camera control strategies
│   ├── CameraConstants.hpp # Camera configuration constants
│   ├── InputSystem.hpp    # Input handling
│   ├── GridRenderer.hpp   # Grid rendering
│   └── TriangleRenderer.hpp # Triangle renderer (example)
│
├── src/                   # Implementation files
│   ├── main.cpp           # Entry point
│   ├── VkApp.cpp          # Application implementation
│   ├── core/              # Core systems
│   │   ├── VulkanCore.cpp
│   │   ├── VulkanSwapchain.cpp
│   │   ├── Camera.cpp
│   │   ├── CameraController.cpp
│   │   └── InputSystem.cpp
│   └── renderer/          # Renderers
│       ├── GridRenderer.cpp
│       └── TriangleRenderer.cpp
│
├── shaders/               # Slang shader sources
│   ├── Grid.slang         # Grid visualization shader
│   └── Triangle.slang     # Example triangle shader
│
├── cmake/                 # CMake modules
│   └── FindVulkan.cmake   # Vulkan SDK finder
│
├── docs/                  # Documentation
│   └── getting_started.md # Setup instructions
│
└── build/                 # Build output (generated)
    ├── bin/               # Compiled executables
    └── shaders/           # Compiled SPIR-V shaders
```

## Prerequisites

### Required Software

- **CMake** 3.12 or higher
- **C++17 compatible compiler**:
  - GCC 7+ (Linux)
  - Clang 5+ (macOS/Linux)
  - MSVC 2017+ (Windows)
- **Vulkan SDK** 1.2 or higher
  - Download from [LunarG](https://vulkan.lunarg.com/)
- **GLFW** 3.3 or higher
- **Slang Shader Compiler**
  - Download from [GitHub](https://github.com/shader-slang/slang)
- **GLM** (OpenGL Mathematics)

### Platform-Specific Notes

#### macOS
- Requires MoltenVK (included with Vulkan SDK)
- May need Xcode Command Line Tools

#### Windows
- Ensure Vulkan SDK is in system PATH
- MSVC runtime library set to MultiThreadedDLL

#### Linux
- Install Vulkan drivers for your GPU
- May need additional development packages:
  ```bash
  sudo apt-get install libvulkan-dev libglfw3-dev libglm-dev
  ```

## Building the Project

### 1. Clone the Repository

```bash
git clone https://github.com/madhav-sunil/vulkan-cmake-app.git
cd vulkan-cmake-app
```

### 2. Install Dependencies

Ensure all prerequisites are installed. Verify installations:

```bash
# Check CMake
cmake --version

# Check Vulkan SDK
vulkaninfo | head -n 20

# Check Slang compiler
slangc --version
```

### 3. Configure and Build

```bash
# Create build directory
mkdir build
cd build

# Configure (Release build)
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . --config Release

# Or for development with debug symbols and sanitizers
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

### Build Options

- `CMAKE_BUILD_TYPE`: `Debug` or `Release`
- `BUILD_TESTS`: Enable unit tests (default: OFF)

Example:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
```

## Running the Application

From the build directory:

```bash
# Linux/macOS
./bin/vulkan-cmake-app

# Windows
.\bin\Debug\vulkan-cmake-app.exe
```

Or run from the project root:
```bash
./build/bin/vulkan-cmake-app
```

## Controls

### Camera Movement (Free Camera Mode)
- **W/S**: Move forward/backward
- **A/D**: Strafe left/right
- **E/Q**: Move up/down
- **Left Shift**: Sprint (hold for faster movement)
- **Mouse**: Look around (when captured)
- **Tab**: Toggle mouse capture
- **Scroll Wheel**: Adjust field of view (zoom)
- **Escape**: Exit application

## Configuration

### Camera Constants

Edit [`include/CameraConstants.hpp`](include/CameraConstants.hpp) to customize camera behavior:

```cpp
namespace CameraConstants::Defaults {
    constexpr glm::vec3 FREE_CAMERA_POSITION = glm::vec3(0.0f, 2.0f, 5.0f);
    constexpr float FREE_CAMERA_MOVE_SPEED = 5.0f;
    constexpr float FREE_CAMERA_SENSITIVITY = 0.1f;
    // ... more settings
}
```

### Input Bindings

Customize key bindings in [`src/core/InputSystem.cpp`](src/core/InputSystem.cpp):

```cpp
bindKey(GLFW_KEY_W, InputAction::MoveForward, 1.0f);
bindKey(GLFW_KEY_S, InputAction::MoveForward, -1.0f);
// Add custom bindings
```

### Grid Appearance

Modify grid scale in [`src/VkApp.cpp`](src/VkApp.cpp):

```cpp
float gridScale = 0.1f; // Adjust for different grid sizes
```

## Architecture Overview

### Core Systems

- **[`VulkanCore`](include/VulkanCore.hpp)**: Manages Vulkan instance, device, and swapchain
- **[`VulkanSwapchain`](include/VulkanSwapchain.hpp)**: Handles swapchain creation and recreation
- **[`Camera`](include/Camera.hpp)**: View and projection matrix management
- **[`InputSystem`](include/InputSystem.hpp)**: Unified input handling with action mapping
- **[`CameraController`](include/CameraController.hpp)**: Strategy pattern for camera control modes

### Renderers

Renderers implement a simple interface:
- `recordCommands(VkCommandBuffer cmd, ...)`: Record draw commands
- `resize(VkExtent2D extent)`: Handle window resize

Add new renderers by:
1. Creating a class in `include/` and `src/renderer/`
2. Implementing the render interface
3. Instantiating in [`VkApp::initialize()`](src/VkApp.cpp)
4. Calling `recordCommands()` in [`VkApp::run()`](src/VkApp.cpp)

## Shader Development

Shaders are written in **Slang** and compiled to SPIR-V at build time.

### Adding New Shaders

1. Create a `.slang` file in `shaders/`
2. Add compilation targets in [`CMakeLists.txt`](CMakeLists.txt):

```cmake
set(MY_SHADER ${CMAKE_SOURCE_DIR}/shaders/MyShader.slang)
set(MY_VERT_SPV ${CMAKE_BINARY_DIR}/shaders/my_shader.vert.spv)
set(MY_FRAG_SPV ${CMAKE_BINARY_DIR}/shaders/my_shader.frag.spv)

add_custom_command(
    OUTPUT ${MY_VERT_SPV} ${MY_FRAG_SPV}
    COMMAND ${SLANGC_EXECUTABLE} -target spirv -entry vs_main -stage vertex -o ${MY_VERT_SPV} ${MY_SHADER}
    COMMAND ${SLANGC_EXECUTABLE} -target spirv -entry ps_main -stage fragment -o ${MY_FRAG_SPV} ${MY_SHADER}
    DEPENDS ${MY_SHADER}
    COMMENT "Compiling MyShader.slang"
)
```

### Shader Hot Reload

Shaders are recompiled on build. To see changes:
1. Modify shader source
2. Rebuild project
3. Restart application

## Troubleshooting

### Vulkan SDK Not Found
```bash
export VULKAN_SDK=/path/to/vulkan/sdk
export PATH=$VULKAN_SDK/bin:$PATH
export LD_LIBRARY_PATH=$VULKAN_SDK/lib:$LD_LIBRARY_PATH  # Linux
export DYLD_LIBRARY_PATH=$VULKAN_SDK/lib:$DYLD_LIBRARY_PATH  # macOS
```

### Slang Compiler Not Found
Ensure `slangc` is in your PATH or set `SLANGC_EXECUTABLE` in CMake:
```bash
cmake -DSLANGC_EXECUTABLE=/path/to/slangc ..
```

## Development

### Adding Features

1. **New Renderer**: Inherit from base pattern (see [`GridRenderer`](include/GridRenderer.hpp))
2. **Camera Mode**: Implement [`CameraController`](include/CameraController.hpp) interface
3. **Input Action**: Add to [`InputAction`](include/InputSystem.hpp) enum and bind keys

### Debugging

Enable sanitizers in Debug builds (non-MSVC):
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
```
Includes Address Sanitizer and Undefined Behavior Sanitizer.

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **Vulkan Tutorial**: [vulkan-tutorial.com](https://vulkan-tutorial.com/)
- **Slang**: [shader-slang.com](https://shader-slang.com/)
- **GLFW**: [glfw.org](https://www.glfw.org/)
- **GLM**: [glm.g-truc.net](https://glm.g-truc.net/)

## Resources

- [Vulkan Specification](https://www.khronos.org/registry/vulkan/)
- [Vulkan Guide](https://github.com/KhronosGroup/Vulkan-Guide)
- [Slang Documentation](https://shader-slang.com/slang/user-guide/)
- [Getting Started Guide](docs/getting_started.md)

## Contact

For questions or issues, please open an issue on GitHub.

---

**Note**: This is a learning/demonstration project. For production use, consider additional features like:
- Proper error handling and recovery
- Resource pooling and caching
- Multi-threaded rendering
- Descriptor set management
- Memory allocators (VMA)