add_executable(qreplnode
    main.cc
    QReplNode.cc
)

target_link_libraries(qreplnode
    qdht
    qhttp
    boost_filesystem
    boost_regex
    boost_system
    capnp
    kj
    log
    log4cxx
    pthread
)
