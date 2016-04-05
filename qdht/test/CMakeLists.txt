add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/testqdht.capnp.h ${CMAKE_CURRENT_BINARY_DIR}/testqdht.capnp.c++
    COMMAND capnp compile -oc++ --src-prefix=${CMAKE_SOURCE_DIR}/qdht/test ${CMAKE_SOURCE_DIR}/qdht/test/testqdht.capnp
    DEPENDS testqdht.capnp
)

add_executable(testqdht
    main.cc
    testId.cc
    testNode.cc
    testqdht.capnp.c++
)

target_link_libraries(testqdht
    qdht
    boost_system
    capnp
    pthread
)

add_test(
    NAME testqdht
    COMMAND testqdht
)
