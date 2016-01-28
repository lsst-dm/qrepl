#ifndef LSST_QSERV_HTTP_REQUEST_HPP
#define LSST_QSERV_HTTP_REQUEST_HPP

// System headers
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Third-party headers
#include "boost/asio.hpp"

namespace lsst {
namespace qserv {
namespace qhttp {

class Request : public std::enable_shared_from_this<Request>
{
public:

    using Ptr = std::shared_ptr<Request>;

    boost::asio::ip::tcp::endpoint localAddr;

    std::string method;   // HTTP header method
    std::string target;   // HTTP header target
    std::string version;  // HTTP header version

    std::string path;                                      // path portion of URL
    std::unordered_map<std::string, std::string> query;    // parsed URL query parameters
    std::unordered_map<std::string, std::string> header;   // parsed HTTP headers
    std::unordered_map<std::string, std::string> params;   // captured URL path elements

    std::istream content;                                  // unparsed body
    std::unordered_map<std::string, std::string> body;     // parsed body, if x-www-form-urlencoded

private:

    friend class Server;

    Request(Request const&) = delete;
    Request& operator=(Request const&) = delete;

    explicit Request(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    void _parseHeader();
    void _parseUri();
    void _parseBody();

    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    boost::asio::streambuf _requestbuf;

};

}}} // namespace lsst::qserv::qhttp

#endif // LSST_QSERV_HTTP_REQUEST_HPP
