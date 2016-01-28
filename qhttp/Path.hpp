#ifndef LSST_QSERV_HTTP_PATH_HPP
#define LSST_QSERV_HTTP_PATH_HPP

// System headers
#include <string>
#include <vector>

// Third-party headers
#include "boost/regex.hpp"

// Local headers
#include "qhttp/Request.hpp"

namespace lsst {
namespace qserv {
namespace qhttp {

//
// ----- This is an internal utility class, used by the Server class, that encapsulates compiling a path
//       specifier into a matching regexp, and then updating and captured params in a Request after matching
//       against the compiled regexp.  The internals of this are a fairly straight port of path-to-regexp
//       (https://github.com/pillarjs/path-to-regexp), as used by express.js; see that link for
//       examples of supported path syntax.
//

class Path
{
public:

    void parse(const std::string &pattern);
    void updateParamsFromMatch(Request::Ptr const& request, boost::smatch const& pathMatch);

    boost::regex regex;
    std::vector<std::string> paramNames;

};

}}} // namespace lsst::qserv::qhttp

#endif // LSST_QSERV_HTTP_PATH_HPP
