// System headers
#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/test/unit_test.hpp"
#include "capnp/message.h"

// Local headers
#include "qdht/Client.hpp"
#include "qdht/Id.hpp"
#include "qdht/Node.hpp"
#include "qdht/qdht.capnp.h"
#include "qdht/test/testqdht.capnp.h"

namespace asio = boost::asio;
namespace ip = boost::asio::ip;

using namespace std;

namespace lsst {
namespace qserv {

BOOST_AUTO_TEST_SUITE(Node)

//
//----- The test fixture holds a boost::asio::io_service, and manages a single thread to run
//      its event loop.
//

struct QdhtFixture
{
    QdhtFixture()
    {
    }

    void startAsio()
    {
        serviceThread = std::thread([this](){ service.run(); });
    }

    ~QdhtFixture()
    {
        service.stop();
        serviceThread.join();
    }

    asio::io_service service;
    std::thread serviceThread;
};

//
//----- Test helper encapsulates plumbing to send a qdht message, track its routing and
//      verify delivery with a timeout.  Uses the Test capnp message class, defined in
//      testqdht.capnp.  Make an instance of this for each test message; perform() is
//      asynchronous so multiple messages may be distpatched in parallel, then validate()
//      should subsequently be called on each.
//

struct QdhtTestHelper
{
    //----- All QdhtTestHelper instances are assigned and ID which can be resolved in a class static map

    uint32_t helperId;

    static uint32_t nextHelperId;
    static std::map<uint32_t, QdhtTestHelper*> helpersById;

    QdhtTestHelper(asio::io_service& service)
    :
        service(service),
        deliveryTimeout(service)
    {
        helperId = nextHelperId++;
        helpersById[helperId] = this;
    }

    ~QdhtTestHelper()
    {
        helpersById.erase(helperId);
    }

    QdhtTestHelper& perform(qdht::Node::Ptr const& node, qdht::Id const& target, string const& content)
    {
        //----- Create top-level qdht message, and fill in client and target

        delivery d{node, std::unique_ptr<capnp::MallocMessageBuilder>(new capnp::MallocMessageBuilder)};
        deliveryTrace.emplace_back(std::move(d));
        qdht::proto::Message::Builder msg = deliveryTrace.back().msg->initRoot<qdht::proto::Message>();
        msg.setClientId(lsst::qserv::TEST_CLIENT);
        msg.initTarget(target.size());
        target.serialize(msg.getTarget().begin());

        //----- Cast body to lsst::qserv::Test message type, and populate

        auto testMsg = msg.getBody().getAs<lsst::qserv::Test>();
        testMsg.setHelperId(helperId);
        testMsg.setContent(content);

        //----- Setup delivery timeout

        deliveryTimeout.expires_from_now(std::chrono::seconds(1));
        deliveryTimeout.async_wait([this](boost::system::error_code const& ec) {
            if (!ec) {
                std::unique_lock<std::mutex> lock(doneMutex);
                done = true;
                deliveryTimedOut = true;
                lock.unlock();
                doneCv.notify_one();
            }
        });

        //----- Send the message using the qdht API

        BOOST_TEST(node->send(msg).value() == boost::system::errc::success);

        return *this;
    }

    QdhtTestHelper& validate(qdht::Id const& target)
    {
        //----- Wait for the message to be delivered or timeout

        std::unique_lock<std::mutex> lock(doneMutex);
        doneCv.wait(lock, [this]{ return done; });
        BOOST_TEST(!deliveryTimedOut);

        //----- Was the message delivered to the intended target?

        BOOST_TEST(deliveryTrace.back().node->local().id == target);

        //----- Did the message make it across the wire intact?

        auto sent = deliveryTrace.front().msg->getRoot<qdht::proto::Message>().asReader();
        auto sentTest = sent.getBody().getAs<lsst::qserv::Test>();

        auto recd = deliveryTrace.back().msg->getRoot<qdht::proto::Message>().asReader();
        auto recdTest = recd.getBody().getAs<lsst::qserv::Test>();

        BOOST_TEST(std::string(recdTest.getContent()) == std::string(sentTest.getContent()));

        return *this;
    }

    static void registerClient(qdht::Node::Ptr const& node)
    {
        qdht::Client client;

        client.clientId = lsst::qserv::TEST_CLIENT;

        //----- Delivery callback.  Dig the helper ID out of the message, and lookup the helper instance.
        //      Cancel timeout timer on helper, record delivery in its trace, and signal delivery.
        //      Closes over shared ptr to node on which the callback is being installed.

        client.onDeliver = [node](qdht::proto::Message::Reader msg) {
            auto test = msg.getBody().getAs<lsst::qserv::Test>();
            auto ih = helpersById.find(test.getHelperId());
            BOOST_TEST((ih != helpersById.end()));
            if (ih != helpersById.end()) {
                auto& h = *(ih->second);
                h.deliveryTimeout.cancel();
                std::unique_lock<std::mutex> lock(h.doneMutex);
                h.done = true;
                h.deliveryTimedOut = false;
                lock.unlock();
                delivery d{node, std::unique_ptr<capnp::MallocMessageBuilder>(new capnp::MallocMessageBuilder)};
                h.deliveryTrace.emplace_back(std::move(d));
                h.deliveryTrace.back().msg->setRoot(msg);
                h.doneCv.notify_one();
            }
        };

        node->addClient(client);
    }

    //----- Delivery trace tracks the routing and delivery of our message for subsequent validation.
    //      A vector is used because we plan to trace routing through intermediate nodes in the future.

    struct delivery {
        qdht::Node::Ptr node;
        std::unique_ptr<capnp::MallocMessageBuilder> msg;
    };

    std::vector<delivery> deliveryTrace;

    //----- Condition variable so validate() can wait for delivery or timeout

    bool done = {false};
    std::mutex doneMutex;
    std::condition_variable doneCv;

    //----- Delivery timeout timer that runs via the asio io_service

    asio::io_service& service;
    asio::steady_timer deliveryTimeout;
    bool deliveryTimedOut = {false};
};


uint32_t QdhtTestHelper::nextHelperId {0};
std::map<uint32_t, QdhtTestHelper*> QdhtTestHelper::helpersById;


BOOST_FIXTURE_TEST_CASE(node_single, QdhtFixture)
{
    //---- Create a single node and install its handlers on the asio::io_service in the test fixture

    qdht::RoutePoint local;
    auto node = qdht::Node::create(service, local);
    QdhtTestHelper::registerClient(node);
    BOOST_TEST(node->installAsioHandlers().value() == boost::system::errc::success);

    //----- Start the asio::io_service in the test fixture

    startAsio();

    //----- Create some helpers and dispatch some messages.  Since this is a single node test,
    //      messages to all targets should be delivered to the single node.

    QdhtTestHelper ha(service), hb(service), hc(service), hd(service), he(service), hf(service);

    qdht::Id a, b, c, d, e, f;

    a.parse("0000000000000000000000000000000000000000");
    b.parse("0000000000000000000000000000000000000001");
    c.parse("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    d.parse("8000000000000000000000000000000000000000");
    e.parse("8000000000000000000000000000000000000001");
    f.parse("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

    ha.perform(node, a, "a");
    hb.perform(node, b, "b");
    hc.perform(node, c, "c");
    hd.perform(node, d, "d");
    he.perform(node, e, "e");
    hf.perform(node, f, "f");

    ha.validate(local.id);
    hb.validate(local.id);
    hc.validate(local.id);
    hd.validate(local.id);
    he.validate(local.id);
    hf.validate(local.id);
}


BOOST_AUTO_TEST_SUITE_END()

}} // namespace lsst::qserv

