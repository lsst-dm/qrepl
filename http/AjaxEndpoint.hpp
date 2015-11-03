#ifndef LSST_QSERV_HTTP_AJAXENDPOINT_HPP
#define LSST_QSERV_HTTP_AJAXENDPOINT_HPP

// System headers
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Local headers
#include "http/Response.hpp"

namespace lsst {
namespace qserv {
namespace http {

class Server;

class AjaxEndpoint
{
public:

    using Ptr = std::shared_ptr<AjaxEndpoint>;

    static Ptr add(Server& server, std::string const& path);
    void update(std::string const& json);

private:

    friend class Server;

    std::vector<Response::Ptr> _pendingResponses;
    std::mutex _pendingResponsesMutex;

};

}}} // namespace lsst::qserv::http

#endif // LSST_QSERV_HTTP_AJAXENDPOINT
