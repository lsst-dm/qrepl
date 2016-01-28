#ifndef LSST_QSERV_HTTP_AJAXENDPOINT_HPP
#define LSST_QSERV_HTTP_AJAXENDPOINT_HPP

// System headers
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// Local headers
#include "qhttp/Response.hpp"

namespace lsst {
namespace qserv {
namespace qhttp {

class Server;

class AjaxEndpoint
{
public:

    using Ptr = std::shared_ptr<AjaxEndpoint>;

    static Ptr add(Server& server, std::string const& path);
    void update(std::string const& json); // thread-safe

private:

    friend class Server;

    AjaxEndpoint();

    std::vector<Response::Ptr> _pendingResponses;
    std::mutex _pendingResponsesMutex;

};

}}} // namespace lsst::qserv::qhttp

#endif // LSST_QSERV_HTTP_AJAXENDPOINT
