// Class header
#include "qdht/Leafset.hpp"
#include "qdht/RoutePoint.hpp"
#include "qdht/qdht.capnp.h"

namespace lsst {
namespace qserv {
namespace qdht {


Leafset::Leafset()
{
}


Leafset::~Leafset()
{
}


RouteDisposition Leafset::route(proto::Message::Reader msg, RoutePoint& nextHop)
{
    return RouteDisposition::DELIVER_LOCAL;
}


}}} // namespace lsst::qserv::qdht
