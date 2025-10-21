# Vulkan CMake Application

## Overview
This project is a Vulkan-based application that demonstrates the use of the Vulkan graphics API. It is structured using CMake for build configuration and includes a simple rendering example of a triangle.

## Project Structure
```
vulkan-cmake-app
├── CMakeLists.txt          # Main CMake configuration file
├── cmake                   # Custom CMake modules
│   └── FindVulkan.cmake    # Logic to find Vulkan SDK
├── include                 # Header files
│   └── vk_app.hpp          # Declaration of the VkApp class
├── src                     # Source files
│   ├── CMakeLists.txt      # CMake configuration for source files
│   ├── main.cpp            # Entry point of the application
│   ├── app.cpp             # Implementation of VkApp methods
│   └── renderer.cpp        # Rendering logic
├── shaders                 # Shader files
│   ├── triangle.vert       # Vertex shader for triangle
│   └── triangle.frag       # Fragment shader for triangle
├── tests                   # Test files
│   ├── CMakeLists.txt      # CMake configuration for tests
│   └── test_renderer.cpp    # Unit tests for rendering logic
├── docs                    # Documentation
│   └── getting_started.md   # Setup and usage instructions
├── .gitignore              # Files to ignore by Git
├── LICENSE                 # Licensing information
└── README.md               # Project documentation
```

## Setup Instructions
1. **Clone the repository:**
   ```
   git clone <repository-url>
   cd vulkan-cmake-app
   ```

2. **Install dependencies:**
   Ensure you have the Vulkan SDK installed on your system. Follow the instructions on the official Vulkan website for your platform.

3. **Build the project:**
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **Run the application:**
   After building, you can run the application with:
   ```
   ./vulkan-cmake-app
   ```

## Usage
The application initializes a Vulkan instance, sets up a rendering pipeline, and displays a triangle on the screen. You can modify the shaders in the `shaders` directory to experiment with different rendering techniques.

## Contributing
Contributions are welcome! Please open an issue or submit a pull request for any improvements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.