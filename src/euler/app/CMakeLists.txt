add_library(euler_app STATIC
        state.cpp
        state.h
)

if (EULER_DRAGONRUBY_BUILD)
    target_sources(euler_app PUBLIC
            dragonruby/logger.cpp
            dragonruby/logger.h
            dragonruby/ruby_state.cpp
            dragonruby/ruby_state.h
            dragonruby/state.cpp
            dragonruby/state.h
            dragonruby/image.cpp
            dragonruby/image.h
            dragonruby/window.cpp
            dragonruby/window.h
            dragonruby/target.cpp
            dragonruby/target.h
    )
    target_link_libraries(euler_app PUBLIC
            dragonruby
            earcut_hpp
    )
    target_include_directories(euler_app PUBLIC
            ${EULER_DRAGONRUBY_PATH}/include
    )
    target_compile_definitions(euler_app PUBLIC
            -DEULER_DRAGONRUBY
    )
elseif (EULER_NATIVE_BUILD)
    target_sources(euler_app PUBLIC
            native/logger.cpp
            native/logger.h
            native/state.cpp
            native/state.h
            native/ruby_state.cpp
            native/ruby_state.h
    )
    add_executable(euler
            native/main.cpp
    )
    target_link_libraries(euler PUBLIC
            euler_app
    )
    target_link_libraries(euler_app PUBLIC
            euler_vulkan
            quill
    )
endif ()

target_link_libraries(euler_app PUBLIC
        euler_util
)

if (EULER_PHYSICS_BUILD)
    target_link_libraries(euler_app PUBLIC
            euler_physics
    )
endif ()

if (EULER_GRAPHICS_BUILD)
    target_link_libraries(euler_app PUBLIC
            euler_graphics
    )
endif ()

if (EULER_NET_BUILD)
    target_link_libraries(euler_app PUBLIC
            euler_net
    )
endif ()