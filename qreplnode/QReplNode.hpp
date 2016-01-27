#ifndef LSST_QREPL_NODEMGR_HPP
#define LSST_QREPL_NODEMGR_HPP

// System headers
#include <memory>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "capnp/common.h"

// Local headers
#include "qdht/Node.hpp"
#include "qhttp/Request.hpp"
#include "qhttp/Response.hpp"
#include "qhttp/Server.hpp"

namespace lsst {
namespace qserv {
namespace qrepl {

class QReplNode
{
public:

    QReplNode(boost::asio::io_service& ioService);
    ~QReplNode();

    boost::system::error_code init(std::string const& status);

private:

    boost::asio::io_service& _ioService;

    std::shared_ptr<boost::asio::ip::udp::socket> _sendSocket;
    boost::asio::ip::udp::endpoint _sendEndpoint;
    capnp::word _sendBuffer[1024];
    std::shared_ptr<boost::asio::steady_timer> _sendTimer;

    qhttp::Server::Ptr _httpServer;

    qdht::Node::Ptr _node;

    void _sendStatus();

    void _addKey(qhttp::Request::Ptr request, qhttp::Response::Ptr response);
    void _killKey(qhttp::Request::Ptr request, qhttp::Response::Ptr response);

};

}}} // namespace lsst::qserv::qrepl

#endif // LSST_QREPL_NODEMGR_HPP
