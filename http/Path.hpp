#ifndef LSST_QSERV_HTTP_PATH_HPP
#define LSST_QSERV_HTTP_PATH_HPP

// System headers
#include <string>
#include <vector>

// Third-party headers
#include "boost/regex.hpp"

// Local headers
#include "http/Request.hpp"

namespace lsst {
namespace qserv {
namespace http {

class Path
{
public:

    void parse(const std::string &pattern);
    void updateParamsFromMatch(Request::Ptr const& request, boost::smatch const& pathMatch);

    boost::regex regex;
    std::vector<std::string> paramNames;

};

}}} // namespace lsst::qserv::http

#endif // LSST_QSERV_HTTP_PATH_HPP
