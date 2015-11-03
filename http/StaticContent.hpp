#ifndef LSST_QSERV_HTTP_STATICCONTENT_HPP
#define LSST_QSERV_HTTP_STATICCONTENT_HPP

// System headers
#include <string>

// Local headers
#include "http/Server.hpp"

namespace lsst {
namespace qserv {
namespace http {

class StaticContent
{
public:

    static void add(Server& server, std::string const& path, std::string const& rootDirectory);

};

}}} // namespace lsst::qserv::http

#endif // LSST_QSER_HTTP_STATICCONTENT_HPP
