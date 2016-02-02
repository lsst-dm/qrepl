#ifndef LSST_QSERV_QDHT_NODE_HPP
#define LSST_QSERV_QDHT_NODE_HPP

// System headers
#include <memory>
#include <string>
#include <vector>

// Third-party headers
#include "boost/asio.hpp"

// Local headers
#include "qdht/Id.hpp"

namespace lsst {
namespace qserv {
namespace qdht {

class Node
{
public:

    using Ptr = std::shared_ptr<Node>;

    static Ptr create(boost::asio::io_service& ioService, Id const& id);
    ~Node();

    Id const& id() const { return _id; };
    std::vector<Id> const& keys() const { return _keys; };

private:

    explicit Node(boost::asio::io_service& ioService, Id const& id);

    boost::asio::io_service& _ioService;

    Id _id;
    std::vector<Id> _keys;

};

}}} // namespace lsst::qserv::qdht

#endif // LSST_QSERV_QDHT_NODE_HPP
