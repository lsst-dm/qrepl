// Class header
#include "qdht/Node.hpp"

// System headers
#include <memory>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"

// Local headers
#include "qdht/RoutePoint.hpp"

namespace errc = boost::system::errc;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;

namespace lsst {
namespace qserv {
namespace qdht {


Node::Node(boost::asio::io_service& io_service, RoutePoint const& local)
:
    _io_service(io_service),
    _acceptor(io_service, local.endpoint),
    _local(local)
{
}


Node::~Node()
{
}


Node::Ptr Node::create(boost::asio::io_service& io_service, RoutePoint const& local)
{
    return std::shared_ptr<Node>(new Node(io_service, local));
}


boost::system::error_code Node::installAsioHandlers()
{
    boost::system::error_code ec;
    accept();
    return ec;
}


boost::system::error_code Node::cancelAsioHandlers()
{
    boost::system::error_code ec;
    _acceptor.cancel(ec);
    return ec;
}


boost::system::error_code Node::joinCluster(RoutePoint const& bootstrap)
{
    boost::system::error_code ec;
    return ec;
}


boost::system::error_code Node::leaveCluster()
{
    boost::system::error_code ec;
    return ec;
}


boost::system::error_code Node::send(qdht::proto::Message::Reader msg)
{
    boost::system::error_code ec;

    // TODO: message routing logic goes here.  Just deliver all messages locally for now.
    for(auto& client : _clients) {
        if (client.clientId == msg.getClientId()) {
            if (client.onDeliver) {
                client.onDeliver(msg);
            }
        }
    }

    return ec;
}


void Node::accept()
{
    auto socket = std::make_shared<ip::tcp::socket>(_io_service);
    _acceptor.async_accept(
        *socket,
        [this, socket](boost::system::error_code const& ec) {
            if (!ec) {
                accept(); // start accept for the next incoming connection
                // TODO incoming message handling goes here...
            }
        }
    );
}


}}} // namespace lsst::qserv::qdht
