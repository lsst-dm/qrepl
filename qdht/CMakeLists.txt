add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qdht.capnp.h ${CMAKE_CURRENT_BINARY_DIR}/qdht.capnp.c++
    COMMAND capnp compile -oc++ --src-prefix=${CMAKE_SOURCE_DIR}/qdht ${CMAKE_SOURCE_DIR}/qdht/qdht.capnp
    DEPENDS qdht.capnp
)

add_library(qdht
    Id.cc
    qdht.capnp.c++
)

add_executable(testqdht
    testqdht.cc
)

target_link_libraries(testqdht
    qdht
)

add_test(
    NAME testqdht
    COMMAND testqdht
)
