// Class header
#include "http/Server.hpp"

// System headers
#include <memory>
#include <string>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/regex.hpp"

// Local headers
#include "http/AjaxEndpoint.hpp"
#include "http/StaticContent.hpp"

namespace lsst {
namespace qserv {
namespace http {


Server::Ptr Server::create(boost::asio::io_service& io_service, unsigned short port)
{
    // can't use std::make_shared on private ctor; do it the old way...
    return std::shared_ptr<Server>(new Server(io_service, port));
}


unsigned short Server::getPort()
{
    return _acceptor.local_endpoint().port();
}


Server::Server(boost::asio::io_service& io_service, unsigned short port)
:
    _io_service(io_service),
    _acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
}


void Server::addHandler(std::string const& method, std::string const& pattern, Handler handler)
{
    auto &handlers = _pathHandlersByMethod[method];
    handlers.resize(handlers.size() + 1);
    auto &phandler = handlers.back();
    phandler.path.parse(pattern);
    phandler.handler = handler;
}


void Server::addHandlers(std::initializer_list<HandlerSpec> const& handlers)
{
    for(auto& handler: handlers) {
        addHandler(handler.method, handler.pattern, handler.handler);
    }
}


void Server::addStaticContent(std::string const& pattern, std::string const& rootDirectory)
{
    StaticContent::add(*this, pattern, rootDirectory);
}


AjaxEndpoint::Ptr Server::addAjaxEndpoint(const std::string& pattern)
{
    return AjaxEndpoint::add(*this, pattern);
}


void Server::accept()
{
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(_io_service);
    _acceptor.async_accept(
        *socket,
        [this, socket](boost::system::error_code const& ec) {
            accept(); // start accept for the next incoming connection
            if (!ec) {
                boost::asio::ip::tcp::no_delay option(true);
                socket->set_option(option);
                _readRequest(socket);
            }
        }
    );
}


void Server::_readRequest(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto timer = std::make_shared<boost::asio::steady_timer>(_io_service);
    timer->expires_from_now(std::chrono::seconds(300));
    timer->async_wait(
        [socket](boost::system::error_code const& ec) {
            if (!ec) {
                boost::system::error_code ec;
                socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                socket->lowest_layer().close();
            }
        }
    );

    auto self = shared_from_this();
    auto reuseSocket = std::make_shared<bool>(false);
    auto request = std::make_shared<Request>(socket);
    auto response = std::make_shared<Response>(
        socket,
        [self, socket, reuseSocket](boost::system::error_code const& ec, std::size_t sent) {
            if (!ec && *reuseSocket) {
                self->_readRequest(socket);
            }
        }
    );

    boost::asio::async_read_until(
        *socket, request->_requestbuf, "\r\n\r\n",
        [self, socket, reuseSocket, request, response, timer](
            boost::system::error_code const& ec, size_t bytesRead)
        {
            if (!ec) {
                size_t bytesBuffered = request->_requestbuf.size() - bytesRead;
                request->_parseHeader();
                request->_parseUri();
                if (request->version == "HTTP/1.1") {
                    *reuseSocket = true;
                }
                if (request->header.count("Content-Length") > 0) {
                    boost::asio::async_read(
                        *socket, request->_requestbuf,
                        boost::asio::transfer_exactly(
                            stoull(request->header["Content-Length"]) - bytesBuffered
                        ),
                        [self, socket, request, response, timer](
                            boost::system::error_code const& ec, size_t)
                        {
                            timer->cancel();
                            if (!ec) {
                                if (request->header["Content-Type"] == "application/x-www-form-urlencoded") {
                                    request->_parseBody();
                                }
                                self->_dispatchRequest(request, response);
                            }
                        }
                    );
                } else {
                    timer->cancel();
                    self->_dispatchRequest(request, response);
                }
            } else {
                timer->cancel();
            }
        }
    );
}


void Server::_dispatchRequest(Request::Ptr request, Response::Ptr response)
{
    auto pathHandlersIt = _pathHandlersByMethod.find(request->method);
    if (pathHandlersIt != _pathHandlersByMethod.end()) {
        boost::smatch pathMatch;
        for(auto& pathHandler : pathHandlersIt->second) {
            if (boost::regex_match(request->path, pathMatch, pathHandler.path.regex)) {
                pathHandler.path.updateParamsFromMatch(request, pathMatch);
                pathHandler.handler(request, response);
                return;
            }
        }
        response->sendStatus(404);
    }
}

}}} // namespace lsst::qserv::http
