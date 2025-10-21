cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0091 NEW)

project(vulkan-cmake-app VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sanitizers for non-MSVC debug builds
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -fno-omit-frame-pointer -g")
endif()

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Collect sources
file(GLOB_RECURSE APP_SRC CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.c")

add_executable(${PROJECT_NAME} ${APP_SRC})

# Include dirs
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Find libraries
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Link
target_link_libraries(${PROJECT_NAME} PRIVATE glfw Vulkan::Vulkan)

# Ensure working directory when running from IDE / ctest
if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
    # prefer modern CMake MSVC runtime selection (CMP0091)
    # set target property or CMAKE_MSVC_RUNTIME_LIBRARY if needed
else()
    set_property(TARGET ${PROJECT_NAME} PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")
endif()

# Optional: build tests if you have a tests/ dir
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS AND EXISTS "${CMAKE_SOURCE_DIR}/tests/CMakeLists.txt")
    enable_testing()
    add_subdirectory(tests)
endif()