// Class-header
#include "AjaxEndpoint.hpp"

// Local headers
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

namespace lsst {
namespace qserv {
namespace qhttp {


AjaxEndpoint::AjaxEndpoint()
{
}


AjaxEndpoint::Ptr AjaxEndpoint::add(Server& server, std::string const& path)
{
    auto aep = std::shared_ptr<AjaxEndpoint>(new AjaxEndpoint);
    server.addHandler("GET", path, [aep](Request::Ptr request, Response::Ptr response) {
        std::lock_guard<std::mutex> lock{aep->_pendingResponsesMutex};
        aep->_pendingResponses.push_back(response);
    });
    return aep;
}


void AjaxEndpoint::update(std::string const& json)
{
    std::lock_guard<std::mutex> lock(_pendingResponsesMutex);
    for(auto pendingResponse : _pendingResponses) {
        pendingResponse->headers["Cache-Control"] = "no-cache";
        pendingResponse->send(json, "application/json");
    }
    _pendingResponses.clear();
}

}}} // namespace lsst::qserv::qhttp
