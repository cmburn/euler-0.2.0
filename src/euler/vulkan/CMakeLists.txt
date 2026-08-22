add_library(euler_vulkan_vma STATIC
        vma.cpp
)

target_link_libraries(euler_vulkan_vma PUBLIC
        GPUOpen::VulkanMemoryAllocator
)

add_library(euler_vulkan STATIC
        compound_shader.cpp
        compound_shader.h
        compute_pipeline.cpp
        compute_pipeline.h
        compute_shader.cpp
        compute_shader.h
        fragment_shader.cpp
        fragment_shader.h
        graphics_pipeline.cpp
        graphics_pipeline.h
        pipeline.cpp
        pipeline.h
        renderer.cpp
        renderer.h
        shader.cpp
        shader.h
        surface.cpp
        surface.h
        swapchain.cpp
        swapchain.h
        vertex_shader.cpp
        vertex_shader.h
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

add_dependencies(euler_vulkan euler_shaders)


target_compile_definitions(euler_vulkan PUBLIC
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL=0
)
