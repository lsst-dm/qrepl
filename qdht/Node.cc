// Class header
#include "qdht/Node.hpp"

namespace lsst {
namespace qserv {
namespace qdht {


Node::Node(boost::asio::io_service& ioService, Id const& id)
:
    _ioService(ioService),
    _id(id)
{
}


Node::~Node()
{
}


Node::Ptr Node::create(boost::asio::io_service& ioService, Id const& id)
{
    return std::shared_ptr<Node>(new Node(ioService, id));
}


}}} // namespace lsst::qserv::qdht
