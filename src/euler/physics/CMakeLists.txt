add_library(euler_physics STATIC
        body.cpp
        body.h
        chain.cpp
        chain.h
        contact.cpp
        contact.h
        distance_joint.cpp
        distance_joint.h
        ext.cpp
        ext.h
        filter_joint.cpp
        filter_joint.h
        joint.cpp
        joint.h
        motor_joint.cpp
        motor_joint.h
        prismatic_joint.cpp
        prismatic_joint.h
        revolute_joint.cpp
        revolute_joint.h
        shape.cpp
        shape.h
        weld_joint.cpp
        weld_joint.h
        wheel_joint.cpp
        wheel_joint.h
        world.cpp
        world.h
        util.cpp
        util.h
)

target_link_libraries(euler_physics PUBLIC
        box2d
        euler_util
)

target_compile_definitions(euler_physics PUBLIC
        ${EULER_DEFINITIONS}
)
target_compile_options(euler_physics PUBLIC
        ${EULER_CFLAGS}
)

#if (EULER_MATH_BUILD)
#    target_link_libraries(euler_physics PUBLIC euler_math)
#endif ()

target_compile_options(euler_physics PUBLIC
        ${EULER_CFLAGS}
)


