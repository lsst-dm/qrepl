// Class header
#include "QReplNode.hpp"

// System headers
#include <algorithm>
#include <chrono>
#include <cstring>
#include <limits>
#include <random>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"
#include "capnp/message.h"
#include "capnp/serialize-packed.h"

// Local headers
#include "qdht/qdht.capnp.h"

using namespace std;

namespace {
    constexpr auto STATUS_UPDATE_PERIOD = chrono::duration<int, milli>(1000);
}

namespace lsst {
namespace qserv {
namespace qrepl {


QReplNode::QReplNode(boost::asio::io_service& ioService)
:
    _ioService(ioService)
{
}


QReplNode::~QReplNode()
{
}


#define HANDLER(REQ, RESP) [this](qhttp::Request::Ptr REQ, qhttp::Response::Ptr RESP)

boost::system::error_code QReplNode::init(string const& status)
{
    boost::system::error_code ec;

    qdht::Id id;
    id.genRandom();
    _node = qdht::Node::create(_ioService, id);

    _httpServer = qhttp::Server::create(_ioService, 0);
    _httpServer->addHandlers({
        {"POST",   "/api/v1/keys",      HANDLER(req, resp){ _addKey(req, resp); }},
        {"DELETE", "/api/v1/keys/:key", HANDLER(req, resp){ _killKey(req, resp); }},
    });
    _httpServer->accept();

    if (!status.empty()) {
        vector<string> statusParts(2);
        boost::split(statusParts, status, boost::is_any_of(":"));
        boost::asio::ip::udp::resolver::query q(statusParts[0], statusParts[1]);
        boost::asio::ip::udp::resolver r(_ioService);
        auto resolved = r.resolve(q, ec);
        if (ec) {
            return ec;
        }
        _sendEndpoint = resolved->endpoint();
        _sendSocket = make_shared<boost::asio::ip::udp::socket>(_ioService, boost::asio::ip::udp::v4());
        _sendTimer = make_shared<boost::asio::steady_timer>(_ioService);
        _sendStatus();
    }

    return ec; // success
}


void QReplNode::_sendStatus()
{
    _sendTimer->expires_from_now(STATUS_UPDATE_PERIOD);
    memset(_sendBuffer, 0, sizeof(_sendBuffer));
    capnp::MallocMessageBuilder message(kj::arrayPtr(_sendBuffer, 1024));
    auto status = message.initRoot<qdht::NodeStatus>();
    status.setCtrlPort(_httpServer->getPort());
    status.setPid(getpid());
    auto const& id = _node->id();
    auto sId = status.initId(40);
    id.serialize(sId.begin());
    auto keys = _node->keys();
    auto msgKeys = status.initKeys(keys.size());
    for(size_t i=0; i<keys.size(); ++i) {
        auto const& key = keys[i];
        auto sKey = msgKeys.init(i, 40);
        key.serialize(sKey.begin());
    }
    auto words = make_shared<kj::Array<capnp::word>>(capnp::messageToFlatArray(message));
    auto bytes = words->asBytes();
    _sendSocket->async_send_to(
        boost::asio::buffer(bytes.begin(), bytes.size()),
        _sendEndpoint,
        [this, words](boost::system::error_code const &ec, size_t bytesWritten) {
            if (!ec) {
                _sendTimer->async_wait(
                    [this](boost::system::error_code const& ec) {
                        if (!ec) {
                            _sendStatus();
                        }
                    }
                );
            }
        }
    );
}


void QReplNode::_addKey(qhttp::Request::Ptr request, qhttp::Response::Ptr response)
{
}


void QReplNode::_killKey(qhttp::Request::Ptr request, qhttp::Response::Ptr response)
{
}


}}} // namespace lsst::qserv::qrepl
