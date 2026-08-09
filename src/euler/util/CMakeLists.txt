add_library(euler_util STATIC
        config.cpp
        config.h
        date_time.cpp
        date_time.h
        error.cpp
        error.h
        ext.cpp
        ext.h
        file.cpp
        file.h
        logger.cpp
        logger.h
        object.cpp
        object.h
        ruby_state.cpp
        ruby_state.h
        state.cpp
        state.h
        storage.cpp
        storage.h
        time.cpp
        time.h
        types.cpp
        types.h
        version.cpp
        version.h
)

target_compile_definitions(euler_util PUBLIC
        ${EULER_DEFINITIONS}
)

target_compile_options(euler_util PUBLIC
        ${EULER_CFLAGS}
)

target_link_libraries(euler_util PUBLIC
        glm
)

# TODO: not this
target_include_directories(euler_util PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../../_deps/eigen-src
        ${CMAKE_CURRENT_BINARY_DIR}/../../_deps/nuklear-src
)

if (EULER_DRAGONRUBY_BUILD)
    target_link_libraries(euler_util PUBLIC
            dragonruby
    )
endif ()

target_link_libraries(euler_util PUBLIC
        mruby
        SDL3-static
)
