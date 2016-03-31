add_executable(testqdht
    main.cc
    testId.cc
)

target_link_libraries(testqdht
    qdht
)

add_test(
    NAME testqdht
    COMMAND testqdht
)
