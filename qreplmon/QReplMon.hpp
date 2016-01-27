#ifndef LSST_QREPL_QREPLMON_HPP
#define LSST_QREPL_QREPLMON_HPP

// System headers
#include <map>
#include <string>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/system/error_code.hpp"
#include "capnp/common.h"
#include "capnp/message.h"

// Local headers
#include "qhttp/Server.hpp"
#include "qdht/Id.hpp"
#include "qdht/qdht.capnp.h"

namespace lsst {
namespace qserv {
namespace qrepl {

class QReplMon
{
public:

    QReplMon(boost::asio::io_service& ioService, unsigned short httpPort);
    ~QReplMon();

    boost::system::error_code init();

private:

    boost::asio::io_service& _ioService;

    unsigned short _httpPort;
    qhttp::Server::Ptr _httpServer;

    struct Node
    {
        unsigned int timeoutCountdown;
        boost::asio::ip::udp::endpoint sender;
        capnp::MallocMessageBuilder status;
    };

    std::map<qdht::Id, Node> _nodesById;
    boost::asio::steady_timer _nodesTimeoutStrobeTimer;
    bool _nodesByIdDirty;

    qhttp::AjaxEndpoint::Ptr _nodesAjax;
    boost::asio::steady_timer _nodesAjaxChokeTimer;
    bool _nodesAjaxChoked;

    capnp::word _statusRecvBuffer[1024];
    boost::asio::ip::udp::socket _statusRecvSocket;
    boost::asio::ip::udp::endpoint _statusRecvSender;
    std::string _statusArg;

    void _getNodes(qhttp::Request::Ptr request, qhttp::Response::Ptr response);
    void _addNode(qhttp::Request::Ptr request, qhttp::Response::Ptr response);
    void _killNode(qhttp::Request::Ptr request, qhttp::Response::Ptr response);

    void _spawn(
        std::initializer_list<const std::string> const& args,
        boost::system::error_code &ec
    );

    void _recvStatus();

    void _nodesTimeoutStrobeWait();

    void _nodesAjaxSend();
    void _nodesAjaxChoke();

    std::string _nodesToJson();

};

}}} // namespace lsst::qserv::qrepl

#endif // LSST_QREPL_QREPLMON_HPP
