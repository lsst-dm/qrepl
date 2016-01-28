find_package(CURL REQUIRED)

include_directories(
	${CURL_INCLUDE_DIR}
)

add_library(qhttp
    AjaxEndpoint.cc
    Path.cc
    Request.cc
    Response.cc
    Server.cc
    StaticContent.cc
)

add_executable(testqhttp
	testqhttp.cc
)

target_link_libraries(testqhttp
    qhttp
    boost_filesystem
    boost_regex
    boost_system
    ${CURL_LIBRARIES}
    pthread
)

add_test(
	NAME testqhttp
	COMMAND testqhttp
)
