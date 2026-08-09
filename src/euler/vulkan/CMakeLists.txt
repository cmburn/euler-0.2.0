add_library(euler_vulkan_vma STATIC
        vma.cpp
)

target_link_libraries(euler_vulkan_vma PUBLIC
        GPUOpen::VulkanMemoryAllocator
)

add_library(euler_vulkan STATIC
        renderer.cpp
        renderer.h
        surface.cpp
        surface.h
        swapchain.cpp
        swapchain.h
        window.cpp
        window.h
)

target_link_libraries(euler_vulkan PUBLIC
        euler_util
        euler_graphics
        euler_vulkan_vma
)

target_link_libraries(euler_vulkan PRIVATE
        vulkan
)


target_compile_definitions(euler_vulkan PUBLIC
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL=0
)
