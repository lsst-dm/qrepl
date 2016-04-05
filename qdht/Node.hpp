#ifndef LSST_QSERV_QDHT_NODE_HPP
#define LSST_QSERV_QDHT_NODE_HPP

// System headers
#include <memory>
#include <string>
#include <vector>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/system/error_code.hpp"

// Local headers
#include "qdht/Client.hpp"
#include "qdht/Id.hpp"
#include "qdht/Leafset.hpp"
#include "qdht/qdht.capnp.h"
#include "qdht/RoutePoint.hpp"

namespace lsst {
namespace qserv {
namespace qdht {


class Node
{
public:

    using Ptr = std::shared_ptr<Node>;

    static Ptr create(boost::asio::io_service& io_service, RoutePoint const& local);
    ~Node();

    //----- Retrieve the local RoutePoint for this Node.  This may be used e.g. if the Node was created
    //      with a RoutePoint port of 0 to retrieve the system-allocated port.

    RoutePoint const& local() const { return _local; }

    //----- Register callbacks onto this Node.  A Client contains a client ID and a set of callbacks, and
    //      will usually make use of private capnp message class.  Multiple Clients may be registered on
    //      a single Node.

    void addClient(Client const& client) { _clients.emplace_back(client); }

    //----- Install or cancel asio handlers for this Node, on the io_service which was provided to create()

    boost::system::error_code installAsioHandlers();
    boost::system::error_code cancelAsioHandlers();

    //----- Join or leave a cluster.  A Node may participate in at most one cluster at a time.

    boost::system::error_code joinCluster(RoutePoint const& bootstrap);
    boost::system::error_code leaveCluster();

    //----- Send a message towards its destination ID

    boost::system::error_code send(proto::Message::Reader msg);

private:

    Node(boost::asio::io_service& io_service, RoutePoint const& local);

    void accept();

    boost::asio::io_service& _io_service;
    boost::asio::ip::tcp::acceptor _acceptor;

    RoutePoint _local;
    std::vector<Client> _clients;
    Leafset _leafset;

};

}}} // namespace lsst::qserv::qdht

#endif // LSST_QSERV_QDHT_NODE_HPP
