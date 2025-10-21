# FindVulkan.cmake

find_path(VULKAN_INCLUDE_DIR NAMES Vulkan/vulkan.h PATHS ${Vulkan_SDK}/include)

find_library(VULKAN_LIBRARY NAMES vulkan-1 PATHS ${Vulkan_SDK}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vulkan DEFAULT_MSG VULKAN_LIBRARY VULKAN_INCLUDE_DIR)

mark_as_advanced(VULKAN_INCLUDE_DIR VULKAN_LIBRARY)