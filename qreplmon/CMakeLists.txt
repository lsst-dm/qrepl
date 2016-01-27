add_executable(qreplmon
    main.cc
    QReplMon.cc
)

target_link_libraries(qreplmon
    qdht
    qhttp
    boost_filesystem
    boost_regex
    boost_system
    capnp
    log
    log4cxx
    kj
    pthread
)
