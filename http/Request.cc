// Class header
#include "http/Request.hpp"

// Third-party headers
#include "boost/regex.hpp"

namespace lsst {
namespace qserv {
namespace http {

Request::Request(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
:
    content(&_requestbuf),
    _socket(socket)
{
    localAddr = _socket->local_endpoint();
}


void Request::_parseHeader()
{
    std::string line;
    static boost::regex reqRe{"^([^ \\r]+) ([^ \\r]+) ([^ \\r]+)\\r$"}; // e.g. "method target version"
    boost::smatch reqMatch;
    if (getline(content, line) && boost::regex_match(line, reqMatch, reqRe)) {
        method = reqMatch[1];
        target = reqMatch[2];
        version = reqMatch[3];
        static boost::regex headerRe{"^([^:\\r]+): ?([^\\r]*)\\r$"}; // e.g. "header: value"
        boost::smatch headerMatch;
        while(getline(content, line) && boost::regex_match(line, headerMatch, headerRe)) {
            header[headerMatch[1]] = headerMatch[2];
        }
    }
}


void Request::_parseUri()
{
    static boost::regex targetRe{"([^\\?#]*)(?:\\?([^#]*))?"}; // e.g. "path[?query]"
    boost::smatch targetMatch;
    if (boost::regex_match(target, targetMatch, targetRe)) {
        path = targetMatch[1];
        static boost::regex queryRe{"([^=&]+)(?:=([^&]*))?"}; // e.g. "key[=value]"
        auto end = boost::sregex_iterator{};
        for(auto i=boost::make_regex_iterator(targetMatch[2].str(), queryRe); i!=end; ++i) {
            query[(*i)[1]] = (*i)[2];
        }
    }
}


void Request::_parseBody()
{
}

}}}  // namespace lsst::qserv::http
