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
namespace http {

class Request : public std::enable_shared_from_this<Request>
{
public:

    using Ptr = std::shared_ptr<Request>;

    Request(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

    boost::asio::ip::tcp::endpoint localAddr;

    std::string method;
    std::string target;
    std::string version;

    std::string path;
    std::unordered_map<std::string, std::string> query;
    std::unordered_map<std::string, std::string> header;

    std::unordered_map<std::string, std::vector<std::string>> params;

    std::istream content;
    std::unordered_map<std::string, std::string> body;

private:

    friend class Server;

    void _parseHeader();
    void _parseUri();
    void _parseBody();

    std::shared_ptr<boost::asio::ip::tcp::socket> _socket;
    boost::asio::streambuf _requestbuf;

};

}}} // namespace lsst::qserv::http

#endif // LSST_QSERV_HTTP_REQUEST_HPP
