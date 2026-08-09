set(SDL_STATIC ON)
set(PHYSFS_BUILD_TEST OFF)
set(BUILD_SHARED_LIBS OFF)

if (EULER_NATIVE_BUILD)
    set(MRUBY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mruby/build/host/include
            CACHE PATH "Path to the mruby include directory" FORCE)

    file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/mruby-headers.txt MRUBY_HEADERS)
    message(STATUS "MRuby headers: ${MRUBY_HEADERS}")

    add_library(mruby INTERFACE
            ${MRUBY_HEADERS}
    )

    set(MRUBY_LIBRARIES
            ${CMAKE_CURRENT_SOURCE_DIR}/mruby/build/host/lib/libmruby.a
            ${CMAKE_CURRENT_SOURCE_DIR}/mruby/build/host/lib/libmruby_core.a
    )

    target_include_directories(mruby INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/mruby/build/host/include
    )

    target_compile_definitions(mruby INTERFACE
            MRB_USE_CXX_EXCEPTION
            MRB_USE_CXX_ABI
            MRB_USE_SET
            HAVE_MRUBY_IO_GEM
            MRB_USE_RATIONAL
            MRB_USE_COMPLEX
            MRB_USE_BIGINT
            MRB_USE_DEBUG_HOOK
    )

    target_link_libraries(mruby INTERFACE ${MRUBY_LIBRARIES})

    if (WIN32)
        target_link_libraries(mruby INTERFACE ws2_32 wsock32)
    endif ()
endif ()

if (EULER_GRAPHICS_BUILD)
    add_subdirectory(glm)
endif ()

if (EULER_GUI_BUILD)
    add_library(nuklear INTERFACE
            Nuklear/nuklear.h
    )
    target_include_directories(nuklear INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/Nuklear
    )
endif ()

if (EULER_NET_BUILD)
    set(Protobuf_USE_STATIC_LIBS ON)
    add_subdirectory(GameNetworkingSockets)
endif ()

if (EULER_WORLD_BUILD)
    add_subdirectory(nlohmann-json)
endif ()

if (EULER_NATIVE_BUILD)
    set(SPDLOG_USE_STD_FORMAT ON)
    set(PHYSFS_BUILD_SHARED OFF)
    set(SDL_STATIC ON)
    list(APPEND PC_PRIVATE_REQUIRES "sdl3")
    add_subdirectory(SDL3)
    # TODO: implement locally?
    add_subdirectory(tinygltf)
    add_subdirectory(KTX-Software/lib)
    add_subdirectory(VulkanMemoryAllocator)
    add_subdirectory(quill)
endif ()