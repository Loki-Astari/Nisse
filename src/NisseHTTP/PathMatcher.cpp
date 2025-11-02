#include "PathMatcher.h"
#include "Request.h"
#include <ThorsLogging/ThorsLogging.h>

using namespace ThorsAnvil::Nisse::HTTP;

std::string PathMatcher::decode(std::string_view matched)
{
    std::string result;
    result.reserve(matched.size());

    for (std::size_t loop = 0; loop < matched.size(); ++loop)
    {
        char n = matched[loop];
        if ((n == '%') && (loop+2) < matched.size())
        {
            char c1 = matched[loop + 1];
            int  v1 = (c1 >= '0' && c1 <= '9') ? c1 - '0' : c1 - 'A' + 10;
            char c2 = matched[loop + 2];
            int  v2 = (c2 >= '0' && c2 <= '9') ? c2 - '0' : c2 - 'A' + 10;
            n = ((v1 % 16) << 4) | (v2 %16);
            loop += 2;
        }
        result += n;
    }
    return result;
}

void PathMatcher::addPath(MethodChoice method, std::string pathMatch, Action&& action)
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addPath", pathMatch);
    MatchList   matchSections;
    NameList    names;

    std::size_t prefix   = 0;
    std::size_t nameBeg  = 0;
    std::size_t nameEnd  = 0;
    std::size_t size     = pathMatch.size();
    bool        first    = true;

    while (prefix != size)
    {
        nameBeg = std::min(size, pathMatch.find('{', prefix));
        nameEnd  = std::min(size, pathMatch.find('}', nameBeg));

        if (!first && prefix == nameBeg) {
            ThorsLogAndThrowDebug("ThorsAnvil::Nisse::HTPP::PathMatcher", "addPath", "Invalid 'pathMatch' string. Multiple name sections with no gap");
        }
        matchSections.emplace_back(pathMatch.substr(prefix, nameBeg - prefix));
        first = false;
        if (nameBeg == size) {
            break;
        }

        if (nameEnd == size) {
            ThorsLogAndThrowDebug("ThorsAnvil::Nisse::HTPP::PathMatcher", "addPath", "Invalid 'pathMatch' string. Badly nested braces.");
        }
        if (nameBeg + 1 == nameEnd) {
            ThorsLogAndThrowDebug("ThorsAnvil::Nisse::HTPP::PathMatcher", "addPath", "Invalid 'pathMatch' string. Name section with no name");
        }

        names.emplace_back(pathMatch.substr(nameBeg + 1, nameEnd - nameBeg - 1));
        prefix = nameEnd + 1;
    }
    if (nameBeg != size) {
        matchSections.emplace_back("");
    }
    paths.emplace_back(method, std::move(matchSections), std::move(names), std::move(action));
}

bool PathMatcher::checkPathMatch(MatchInfo const& pathMatchInfo, std::string_view path, Request& request, Response& response)
{
    // If it is not holding a `Method` it is holding All::Method.
    if (std::holds_alternative<Method>(pathMatchInfo.method) && std::get<Method>(pathMatchInfo.method) != request.getMethod()) {
        return false;
    }

    Match   result;

    std::string_view    prefix = path.substr(0, pathMatchInfo.matchSections[0].size());
    path.remove_prefix(pathMatchInfo.matchSections[0].size());

    if (pathMatchInfo.matchSections[0] != prefix) {
        return false;
    }

    for (std::size_t loop = 1; loop < pathMatchInfo.matchSections.size(); ++loop)
    {
        auto find = pathMatchInfo.matchSections[loop] == "" ? path.size() : path.find(pathMatchInfo.matchSections[loop]);

        if (find == std::string::npos) {
            return false;
        }
        result.emplace(pathMatchInfo.names[loop - 1], decode(path.substr(0, find)));

        path.remove_prefix(find);
        path.remove_prefix(pathMatchInfo.matchSections[loop].size());
    }

    if (!path.empty()) {
        return false;
    }

    pathMatchInfo.action(result, request, response);
    return true;
}

bool PathMatcher::findMatch(std::string_view path, Request& request, Response& response)
{
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::PathMatcher", "findMatch", "Looking for: ", path);
    for (auto const& pathMatchInfo: paths)
    {
        if (checkPathMatch(pathMatchInfo, path, request, response)) {
            ThorsLogDebug("ThorsAnvil::Nisse::HTTP::PathMatcher", "findMatch", "Found");
            return true;
        }
    }
    ThorsLogDebug("ThorsAnvil::Nisse::HTTP::PathMatcher", "findMatch", "No Match");
    return false;
}
