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
namespace http {

class Response : public std::enable_shared_from_this<Response>
{
public:

    using Ptr = std::shared_ptr<Response>;

    using DoneCallback = std::function<void(
        boost::system::error_code const &ec,
        std::size_t bytesTransferred
    )>;

    Response(
        std::shared_ptr<boost::asio::ip::tcp::socket> socket,
        DoneCallback const& doneCallback
    );

    void send(std::string const& content, std::string const& contentType="text/html");
    void sendStatus(unsigned int status);
    void sendFile(boost::filesystem::path const& path);

    unsigned int status = { 200 };
    std::unordered_map<std::string, std::string> headers;

private:

    std::string _headers() const;

    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    boost::asio::streambuf _responsebuf;

    DoneCallback _doneCallback;

};

}}} // namespace lsst::qserv::http

#endif // LSST_QSERV_HTTP_RESPONSE_HPP
