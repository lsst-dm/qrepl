#ifndef LSST_QSERV_HTTP_RESPONSE_HPP
#define LSST_QSERV_HTTP_RESPONSE_HPP

// System headers
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

// Third-party headers
#include "boost/asio.hpp"
#include "boost/filesystem.hpp"

namespace lsst {
namespace qserv {
namespace qhttp {

class Response : public std::enable_shared_from_this<Response>
{
public:

    using Ptr = std::shared_ptr<Response>;

    //----- These methods are used to send a response back to the HTTP client.  When using sendStatus,
    //      an appropriate HTML body will be automatically generated.

    void send(std::string const& content, std::string const& contentType="text/html");
    void sendStatus(unsigned int status);
    void sendFile(boost::filesystem::path const& path);

    //----- Response status code and additional headers may also be set with these members, and will be
    //      included/observed by the send methods above (sendStatus and sendFile will override status set
    //      here, though.)

    unsigned int status = { 200 };
    std::unordered_map<std::string, std::string> headers;

private:

    friend class Server;

    Response(Response const&) = delete;
    Response& operator=(Response const&) = delete;

    using DoneCallback = std::function<void(
        boost::system::error_code const& ec,
        std::size_t bytesTransferred
    )>;

    Response(
        std::shared_ptr<boost::asio::ip::tcp::socket> socket,
        DoneCallback const& doneCallback
    );

    std::string _headers() const;

    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    boost::asio::streambuf _responsebuf;

    DoneCallback _doneCallback;

};

}}} // namespace lsst::qserv::qhttp

#endif // LSST_QSERV_HTTP_RESPONSE_HPP
