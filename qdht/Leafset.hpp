#ifndef LSST_QSERV_QDHT_LEAFSET_HPP
#define LSST_QSERV_QDHT_LEAFSET_HPP

// System headers
#include <vector>

// Local headers
#include "qdht/Id.hpp"
#include "qdht/RoutePoint.hpp"
#include "qdht/qdht.capnp.h"

namespace lsst {
namespace qserv {
namespace qdht {

class Leafset
{
public:

    Leafset();
    ~Leafset();

    RouteDisposition route(proto::Message::Reader msg, RoutePoint& nextHop);

private:

    Id _self;
    std::vector<RoutePoint> _ccw;
    std::vector<RoutePoint> _cw;

};

}}} // namespace lsst::qserv::qdht

#endif // LSST_QSERV_QDHT_LEAFSET_HPP
