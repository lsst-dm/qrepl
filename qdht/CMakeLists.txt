add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qdht.capnp.h ${CMAKE_CURRENT_BINARY_DIR}/qdht.capnp.c++
    COMMAND capnp compile -oc++ --src-prefix=${CMAKE_SOURCE_DIR}/qdht ${CMAKE_SOURCE_DIR}/qdht/qdht.capnp
    DEPENDS qdht.capnp
)

add_library(qdht
    Id.cc
    Node.cc
    qdht.capnp.c++
)

add_subdirectory(test)
