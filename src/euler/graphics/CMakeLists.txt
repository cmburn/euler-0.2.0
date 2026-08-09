add_library(euler_graphics STATIC
        color.cpp
        color.h
        ext.cpp
        ext.h
        font.cpp
        font.h
        image.cpp
        image.h
        image_loader.cpp
        image_loader.h
        math.cpp
        math.h
        render_target.cpp
        render_target.h
        window.cpp
        window.h
)

target_link_libraries(euler_graphics PUBLIC
        euler_util
)
