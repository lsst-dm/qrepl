// Class header
#include "StaticContent.hpp"

// Third-party headers
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"

namespace fs = boost::filesystem;

namespace {

// Utility function used to expand and canonicalize a boost filesystem path.
// Similar to fs::canonical(), but doesn't blow up if the tail
// of the path doesn't exist on disk right now.  Used below to disallow serving
// static content above/outside of root location.

fs::path normalize(fs::path const& path) {
    fs::path result;

    // Start by transforming to an absolute path.
    auto absPath = absolute(path);

    // Walk existing part of the path, then call canonical() on it.  This removes any
    // symlinks, ".", or ".." from existing part, correctly handling the fiddly details
    // about ".." in the possible presence of symlinks.
    auto it = absPath.begin();
    for (; exists(result / *it) && it != absPath.end(); ++it) {
        result /= *it;
    }
    result = canonical(result);

    // Existing part is now absolute, expanded, and symlink free.  Now we can walk and
    // add any remainder, snapping any "." or ".." by simple relative motion within
    // the accumulating result.
    for (; it != absPath.end(); ++it) {
        if (*it == "..") {
            result = result.parent_path();
        } else if (*it == ".") {
            continue;
        } else {
            result /= *it;
        }
    }

    return result;
}

} // anon. namespace

namespace lsst {
namespace qserv {
namespace qhttp {


void StaticContent::add(Server& server, std::string const& pattern, std::string const& rootDirectory)
{
    fs::path rootPath = fs::canonical(rootDirectory);
    server.addHandler("GET", pattern, [rootPath](Request::Ptr request, Response::Ptr response) {
        fs::path requestPath = rootPath;
        requestPath /= request->path;
        requestPath = normalize(requestPath);
        if (!boost::starts_with(requestPath, rootPath)) {
            response->sendStatus(401);
            return;
        }
        if (fs::is_directory(requestPath)) {
            if (!boost::ends_with(request->path, "/")) {
                response->headers["Location"] = request->path + "/";
                response->sendStatus(301);
                return;
            }
            requestPath /= "index.htm";
        }
        response->sendFile(requestPath);
    });
}


}}} // namespace lsst::qserv::qhttp
