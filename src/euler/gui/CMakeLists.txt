add_library(euler_gui_nuklear STATIC
        nuklear.cpp
        nuklear.h
)

target_link_libraries(euler_gui_nuklear PUBLIC
        nuklear
        euler_util
)

add_library(euler_gui STATIC
        button.cpp
        button.h
        checkbox.cpp
        checkbox.h
        color_picker.cpp
        color_picker.h
        common.cpp
        common.h
        context.cpp
        context.h
        dropdown.cpp
        dropdown.h
        element.cpp
        element.h
        ext.cpp
        ext.h
        internal.cpp
        internal.h
        knob.cpp
        knob.h
        progress_bar.cpp
        progress_bar.h
        radio_button.cpp
        radio_button.h
        row.cpp
        row.h
        selectable.cpp
        selectable.h
        slider.cpp
        slider.h
        style.cpp
        style.h
        text.cpp
        text.h
        widget.cpp
        widget.h
        font.cpp
        font.h
        font_atlas.cpp
        font_atlas.h
)

target_link_libraries(euler_gui PUBLIC
        euler_util
        nuklear
)

target_compile_options(euler_gui PUBLIC
        ${EULER_CFLAGS}
)

target_compile_definitions(euler_gui PRIVATE
        ${EULER_DEFINITIONS}
)
