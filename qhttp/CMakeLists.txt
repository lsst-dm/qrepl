add_library(qhttp
    AjaxEndpoint.cc
    Path.cc
    Request.cc
    Response.cc
    Server.cc
    StaticContent.cc
)

add_subdirectory(test)
