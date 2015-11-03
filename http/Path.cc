// Class header
#include "http/Path.hpp"

// System headers
#include <string>
#include <vector>

// Third-party headers
#include "boost/format.hpp"
#include "boost/regex.hpp"
#include "boost/variant.hpp"

namespace {

std::string escapeString(std::string const& str)
{
    static boost::regex escapeRe{"([.[{}()\\*+?|^$])"};
    return regex_replace(str, escapeRe, "\\1");
}

std::string escapeGroup(std::string const& str)
{
    static boost::regex escapeRe{"([=!:$/()])"};
    return regex_replace(str, escapeRe, "\\1");
}

struct PathToken
{
    std::string name;
    std::string prefix;
    std::string delimeter;
    bool optional;
    bool repeat;
    std::string pattern;
};

} // anon. namespace

namespace lsst {
namespace qserv {
namespace http {


void Path::parse(std::string const& pattern)
{
    static boost::regex patternRe{
        // Match escaped characters that would otherwise appear in future matches.
        // This allows the user to escape special characters that won't transform.
        "(\\\\.)|"
        // Match Express-style parameters and un-named parameters with a prefix
        // and optional suffixes. Matches appear as:
        // "/:test(\\d+)?" => [ "/", "test", "\d+", -----, "?", --- ]
        // "/route(\\d+)"  => [ ---, ------, -----, "\d+", ---, --- ]
        // "/*"            => [ "/", ------, -----, -----, ---, "*" ]
        "([\\/.])?(?:(?:\\:(\\w+)(?:\\(((?:\\\\.|[^()])+)\\))?|\\(((?:\\\\.|[^()])+)\\))([+*?])?|(\\*))"
    };

    int key = 0;
    std::string path;
    std::vector<boost::variant<PathToken, std::string>> segments;

    auto end = boost::sregex_iterator{};
    auto last = pattern.begin();
    for(auto i=boost::make_regex_iterator(pattern, patternRe); i!=end; ++i) {
        auto& escaped  = (*i)[1];
        auto& prefix   = (*i)[2];
        auto& name     = (*i)[3];
        auto& capture  = (*i)[4];
        auto& group    = (*i)[5];
        auto& suffix   = (*i)[6];
        auto& asterisk = (*i)[7];

        last = (*i)[0].second;

        // Grab stuff between last match and this
        path.append(i->prefix());

        // Ignore already escaped sequences
        if (escaped.matched) {
            path += escaped.str()[1];
            continue;
        }

        // Push accumulated between stuff onto segment list
        if (!path.empty()) {
            segments.push_back(path);
            path.clear();
        }

        // Add a segment for the matched part
        segments.resize(segments.size() + 1);
        auto &token = boost::get<PathToken>(segments.back());
        token.name = name.matched ? name : std::to_string(key++);
        token.prefix = prefix;
        token.delimeter = prefix.matched ? prefix.str()[0] : '/';
        token.optional = (suffix == "?") || (suffix == "*");
        token.repeat = (suffix == "+") || (suffix == "*");
        token.pattern = escapeGroup(
            capture.matched ? capture
            : group.matched ? group
            : asterisk.matched ? ".*"
            : (std::string("[^") + token.delimeter + "]+?")
        );
    }

    // Scoop up any trailing chars after last match
    if (last != pattern.end()) {
        path.append(last, pattern.end());
    }

    // Then if we have any accumulated path, put it on the token list
    if (!path.empty()) {
        segments.push_back(path);
    }

    std::string route = "^";
    for(auto& segment: segments) {
        if (segment.type() == typeid(std::string)) {
            route += escapeString(boost::get<std::string>(segment));
        } else {
            auto& token = boost::get<PathToken>(segment);
            paramNames.push_back(std::move(token.name));
            std::string prefix = escapeString(token.prefix);
            std::string capture = token.pattern;
            if (token.repeat) {
                capture += str(boost::format("(?:%s%s)*") % prefix % capture);
            }
            if (token.optional) {
                if (!prefix.empty()) {
                    capture = str(boost::format("(?:%s(%s))?") % prefix % capture);
                } else {
                    capture = str(boost::format("(%s)?") % capture);
                }
            } else {
                capture = str(boost::format("%s(%s)") % prefix % capture);
            }
            route += capture;
        }
    }
    route += "(?:\\/(?=$))?$";
    regex = route;
}


void Path::updateParamsFromMatch(Request::Ptr const& request, boost::smatch const& pathMatch)
{
    for(size_t i=0; i<paramNames.size(); ++i) {
        request->params[paramNames[i]].push_back(pathMatch[i+1]);
    }
}


}}} // namespace lsst::qserv::http
