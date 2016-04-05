#ifndef LSST_QSERV_QDHT_CLIENT_HPP
#define LSST_QSERV_QDHT_CLIENT_HPP

// System headers
#include <functional>

// Third-party headers
#include "boost/system/error_code.hpp"

// Local headers
#include "qdht/qdht.capnp.h"

namespace lsst {
namespace qserv {
namespace qdht {

struct Client
{
    uint32_t clientId;
    std::function<void(boost::system::error_code const& ec)> onError;
    std::function<void(proto::Message::Reader msg)> onDeliver;
    std::function<bool(proto::Message::Reader msg)> onForward;
};

}}} // namespace lsst::qserv::qdht

#endif // LSST_QSERV_QDHT_CLIENT_HPP
