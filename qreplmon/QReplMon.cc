// Class header
#include "QReplMon.hpp"

// System headers
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

// Third-party headers
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "capnp/serialize.h"

// Local headers
#include "qdht/Id.hpp"
#include "qdht/qdht.capnp.h"
#include "qhttp/Server.hpp"

namespace pt = boost::property_tree;
using namespace std;

namespace {
    constexpr auto NODES_TIMEOUT_STROBE_PERIOD = std::chrono::duration<int, milli>(100);
    constexpr unsigned int NODES_TIMEOUT_MAX_STROBES = 25;
    constexpr auto NODES_AJAX_CHOKE_TIMEOUT = std::chrono::duration<int, milli>(1100);
}

namespace lsst {
namespace qserv {
namespace qrepl {


QReplMon::QReplMon(boost::asio::io_service& ioService, unsigned short httpPort)
:
    _ioService(ioService),
    _httpPort(httpPort),
    _nodesTimeoutStrobeTimer(ioService),
    _nodesByIdDirty(false),
    _nodesAjaxChokeTimer(ioService),
    _nodesAjaxChoked(false),
    _statusRecvSocket(ioService)
{
}


QReplMon::~QReplMon()
{
}


boost::system::error_code QReplMon::init()
{
    boost::system::error_code ec;

    auto thisHandler = [this](void (QReplMon::*ptmf)(qhttp::Request::Ptr, qhttp::Response::Ptr)) {
        return [this, ptmf](qhttp::Request::Ptr req, qhttp::Response::Ptr resp) {
            (this->*ptmf)(req, resp);
        };
    };

    _httpServer = qhttp::Server::create(_ioService, _httpPort);
    _httpServer->addHandlers({
        {"GET",     "/api/v1/nodes",        thisHandler(&QReplMon::_getNodes) },
        {"POST",    "/api/v1/nodes",        thisHandler(&QReplMon::_addNode)  },
        {"DELETE",  "/api/v1/nodes/:node",  thisHandler(&QReplMon::_killNode) },
    });
    _nodesAjax = _httpServer->addAjaxEndpoint("/api/v1/nodes/ajax");
    _httpServer->addStaticContent("/*", "web");
    _httpServer->accept();

    if (_statusRecvSocket.open(boost::asio::ip::udp::v4(), ec)) return ec;
    if (_statusRecvSocket.bind(boost::asio::ip::udp::endpoint(), ec)) return ec;
    _recvStatus();

    _nodesTimeoutStrobeWait();

    auto statusArg = str(boost::format("--status=127.0.0.1:%1%")
        % _statusRecvSocket.local_endpoint().port());

    _spawn({"qreplnode", statusArg}, ec);
    if (!ec) return ec;

    return ec; // success
}


void QReplMon::_getNodes(qhttp::Request::Ptr request, qhttp::Response::Ptr response)
{
    response->headers["Cache-Control"] = "no-cache";
    response->send(_nodesToJson(), "application/json");
}


void QReplMon::_addNode(qhttp::Request::Ptr request, qhttp::Response::Ptr response)
{
    auto statusArg = str(boost::format("--status=127.0.0.1:%1%")
        % _statusRecvSocket.local_endpoint().port());

    boost::system::error_code ec;
    _spawn({"qreplnode", statusArg}, ec);
    if (ec) {
    }

    std::string trimmedPath = request->path;
    boost::trim_if(trimmedPath, [](char c){ return c == '/'; });

    response->headers["Location"] = str(boost::format("http://%s/%s/%s")
        % request->localAddr % trimmedPath % "");

    response->sendStatus(201);
}


void QReplMon::_killNode(qhttp::Request::Ptr request, qhttp::Response::Ptr response)
{
    qdht::Id id;
    if (!id.parse(request->params["node"])) {
        response->sendStatus(400);
        return;
    }

    auto const& inode = _nodesById.find(id);
    if (inode == _nodesById.end()) {
        response->sendStatus(404);
        return;
    }

    qdht::NodeStatus::Reader const& status = inode->second.status.getRoot<qdht::NodeStatus>();
    kill(status.getPid(), SIGTERM);
    waitpid(status.getPid(), NULL, 0);
    response->sendStatus(204);
}


void QReplMon::_spawn(initializer_list<const string> const& args, boost::system::error_code &ec)
{
    vector<char const*> argv;
    argv.reserve(args.size() + 1);
    for(auto& arg: args) {
        argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);

    auto childPid = fork();
    switch(childPid) {

    case -1:
        ec = boost::system::error_code(errno, boost::system::system_category());
        break;

    case 0:
    {
        if (execvp(argv[0], (char**)&argv[0])) {
            perror("execvp: ");
            exit(errno);
        }
        break; // not reached -- execs or exits above
    }

    default:
        break;

    }
}


bool operator==(qdht::NodeStatus::Reader const& lhs, qdht::NodeStatus::Reader const& rhs) {
//    return (lhs.getId() == rhs.getId())
//        && (lhs.getKeys().size() == rhs.getKeys().size())
//        && equal(lhs.getKeys().begin(), lhs.getKeys().end(), rhs.getKeys().begin());
    return false;
}

bool operator!=(qdht::NodeStatus::Reader const& rhs, qdht::NodeStatus::Reader const& lhs) {
    return !(rhs == lhs);
}


void QReplMon::_recvStatus()
{
    _statusRecvSocket.async_receive_from(
        boost::asio::buffer(_statusRecvBuffer),
        _statusRecvSender,
        [this](boost::system::error_code const& ec, std::size_t bytesRecd) {
            auto words = kj::arrayPtr(_statusRecvBuffer, 1024);
            capnp::FlatArrayMessageReader reader(words);
            auto const& status = reader.getRoot<qdht::NodeStatus>();
            qdht::Id id;
            id.deserialize(status.getId().begin());
            auto& node = _nodesById[id];
            node.timeoutCountdown = NODES_TIMEOUT_MAX_STROBES;
            node.sender = _statusRecvSender;
            if (node.status.getRoot<qdht::NodeStatus>() != status) {
                node.status.setRoot(status);
                _nodesByIdDirty = true;
                _nodesAjaxSend();
            }
            _recvStatus();
        }
    );
}


void QReplMon::_nodesTimeoutStrobeWait()
{
    _nodesTimeoutStrobeTimer.expires_from_now(NODES_TIMEOUT_STROBE_PERIOD);
    _nodesTimeoutStrobeTimer.async_wait(
        [this](boost::system::error_code const &ec) {
            if (!ec) {
                _nodesTimeoutStrobeWait();
                for(auto it=_nodesById.begin(); it!=_nodesById.end();) {
                    if (--(it->second.timeoutCountdown) <= 0) {
                        _nodesById.erase(it++);
                        _nodesByIdDirty = true;
                    } else {
                        ++it;
                    }
                }
                _nodesAjaxSend();
            }
        }
    );
}


void QReplMon::_nodesAjaxSend()
{
    if (_nodesByIdDirty && !_nodesAjaxChoked) {
        _nodesAjax->update(_nodesToJson());
        _nodesByIdDirty = false;
        _nodesAjaxChoke();
    }
}


void QReplMon::_nodesAjaxChoke()
{
    _nodesAjaxChoked = true;
    _nodesAjaxChokeTimer.expires_from_now(NODES_AJAX_CHOKE_TIMEOUT);
    _nodesAjaxChokeTimer.async_wait(
        [this](boost::system::error_code const& ec) {
            if (!ec) {
                _nodesAjaxChoked = false;
                _nodesAjaxSend();
            }
        }
    );
}


string QReplMon::_nodesToJson()
{
    pt::ptree ptree;
    ptree.put_child("data", pt::ptree());
    auto& data = ptree.get_child("data");
    for(auto& node: _nodesById) {
        data.push_back(make_pair("", pt::ptree()));
        auto& jstatus = data.back().second;
        qdht::NodeStatus::Reader const& nstatus = node.second.status.getRoot<qdht::NodeStatus>();
        qdht::Id id;
        id.deserialize(nstatus.getId().begin());
        jstatus.put("id", id);
        jstatus.put_child("keys", pt::ptree());
        auto& keys = jstatus.get_child("keys");
        for(auto const& bkey: nstatus.getKeys()) {
            qdht::Id key;
            key.deserialize(bkey.begin());
            keys.push_back(make_pair("", pt::ptree(key)));
        }
    }
    ostringstream json;
    pt::json_parser::write_json(json, ptree);
    return json.str();
}


}}} // namespace lsst::qserv::qrepl
