#ifndef LSST_QSERV_QDHT_ROUTEPOINT_HPP
#define LSST_QSERV_QDHT_ROUTEPOINT_HPP

// Third-party headers
#include "boost/asio.hpp"

// Local headers
#include "qdht/Id.hpp"

namespace lsst {
namespace qserv {
namespace qdht {

struct RoutePoint
{
    Id id;
    boost::asio::ip::tcp::endpoint endpoint;
};

}}} // namespace lsst::qserv::qdht

#endif // LSST_QSERV_QDHT_ROUTEPOINT_HPP
