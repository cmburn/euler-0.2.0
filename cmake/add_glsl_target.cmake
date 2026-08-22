# add_glsl_target(<target-name> <glsl-file>...)
#
# Creates a custom target named <target-name> that compiles the given GLSL
# shader source files into SPIR-V 1.4 bytecode using glslc. Each input file
# is compiled to "<binary-dir>/<filename>.spv", where <filename> is the
# input file's name (including its GLSL extension, e.g. shader.vert ->
# shader.vert.spv).
#
# The list of generated SPIR-V binaries is available after the call in the
# <target-name>_SPIRV_BINARIES variable, and is also stored on the target
# itself in the SPIRV_BINARY_FILES property.
function (add_glsl_target TARGET_NAME)
    set(glsl_sources ${ARGN})
    if (NOT glsl_sources)
        message(FATAL_ERROR "add_glsl_target(${TARGET_NAME}) requires at least one GLSL source file")
    endif ()

    find_package(Vulkan REQUIRED)
    find_program(GLSLC_EXECUTABLE
            NAMES glslc
            HINTS "$ENV{VULKAN_SDK}/bin" Vulkan::glslc
    )
    if (NOT GLSLC_EXECUTABLE)
        message(FATAL_ERROR "add_glsl_target(${TARGET_NAME}): could not find glslc; is the Vulkan SDK installed?")
    endif ()

    set(spirv_binaries)
    foreach (glsl_source ${glsl_sources})
        get_filename_component(glsl_source_abs "${glsl_source}" ABSOLUTE)
        get_filename_component(glsl_source_name "${glsl_source}" NAME)
        set(spirv_binary "${CMAKE_CURRENT_BINARY_DIR}/${glsl_source_name}.spv")

        add_custom_command(
                OUTPUT "${spirv_binary}"
                COMMAND "${GLSLC_EXECUTABLE}"
                --target-env=vulkan1.4
                --target-spv=spv1.4
                -o "${spirv_binary}"
                "${glsl_source_abs}"
                DEPENDS "${glsl_source_abs}"
                COMMENT "Compiling GLSL shader ${glsl_source_name} to SPIR-V 1.4"
                VERBATIM
        )
        list(APPEND spirv_binaries "${spirv_binary}")
    endforeach ()

    add_custom_target(${TARGET_NAME} ALL DEPENDS ${spirv_binaries})
    set_target_properties(${TARGET_NAME} PROPERTIES SPIRV_BINARY_FILES "${spirv_binaries}")
    set(${TARGET_NAME}_SPIRV_BINARIES "${spirv_binaries}" PARENT_SCOPE)
endfunction()