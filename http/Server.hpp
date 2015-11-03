#ifndef LSST_QSERV_HTTP_SERVER_HPP
#define LSST_QSERV_HTTP_SERVER_HPP

// System headers
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Third-party headers
#include "boost/asio.hpp"

// Local headers
#include "http/AjaxEndpoint.hpp"
#include "http/Path.hpp"
#include "http/Response.hpp"
#include "http/Request.hpp"

namespace lsst {
namespace qserv {
namespace http {

class Server : public std::enable_shared_from_this<Server>
{
public:

    using Ptr = std::shared_ptr<Server>;
    using Handler = std::function<void(Request::Ptr, Response::Ptr)>;

    using HandlerSpec = struct {
        std::string const& method;
        std::string const& pattern;
        Handler handler;
    };

    static Ptr create(boost::asio::io_service& io_service, unsigned short port);
    unsigned short getPort();

    void addHandler(std::string const& method, std::string const& pattern, Handler handler);
    void addHandlers(std::initializer_list<HandlerSpec> const& handlers);

    void addStaticContent(std::string const& path, std::string const& rootDirectory);
    AjaxEndpoint::Ptr addAjaxEndpoint(std::string const& path);

    void accept();

private:

    Server(boost::asio::io_service& io_service, unsigned short port);

    void _readRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void _dispatchRequest(Request::Ptr request, Response::Ptr response);

    struct PathHandler {
        Path path;
        Handler handler;
    };

    std::unordered_map<std::string, std::vector<PathHandler>> _pathHandlersByMethod;

    boost::asio::io_service& _io_service;
    boost::asio::ip::tcp::acceptor _acceptor;

};

}}} // namespace lsst::qserv::http

#endif    // LSST_QSERV_HTTP_SERVER_HPP
