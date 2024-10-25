#include "PathMatcher.h"
#include <ThorsLogging/ThorsLogging.h>

using namespace ThorsAnvil::Nisse::HTTP;

void PathMatcher::addPath(std::string pathMatch, Action&& action)
{
    MatchList   matchSections;
    NameList    names;

    std::size_t prefix   = 0;
    std::size_t nameBeg  = 0;
    std::size_t nameEnd  = 0;
    std::size_t size     = pathMatch.size();
    bool        first    = true;

    // /path1/{name}/{id}
    // Section: >/path1/<
    // Name   : >name<
    // Section: >/<
    // Name   : >id<
    while (prefix != size)
    {
        nameBeg = std::min(size, pathMatch.find('{', prefix));
        nameEnd  = std::min(size, pathMatch.find('}', nameBeg));

        if (!first && prefix == nameBeg) {
            ThorsLogAndThrow("ThorsAnvil::Nisse::HTPP::PathMatcher", "addPath", "Invalid 'pathMatch' string. Multiple name sections with no gap");
        }
        matchSections.emplace_back(pathMatch.substr(prefix, nameBeg - prefix));
        first = false;
        if (nameBeg == size) {
            break;
        }

        if (nameEnd == size) {
            ThorsLogAndThrow("ThorsAnvil::Nisse::HTPP::PathMatcher", "addPath", "Invalid 'pathMatch' string. Badly nested braces.");
        }
        if (nameBeg + 1 == nameEnd) {
            ThorsLogAndThrow("ThorsAnvil::Nisse::HTPP::PathMatcher", "addPath", "Invalid 'pathMatch' string. Name section with no name");
        }

        names.emplace_back(pathMatch.substr(nameBeg + 1, nameEnd - nameBeg - 1));
        prefix = nameEnd + 1;
    }
    if (nameBeg != size) {
        matchSections.emplace_back("");
    }
    paths.emplace_back(std::move(matchSections), std::move(names), std::move(action));
}

bool PathMatcher::checkPathMatch(MatchInfo const& pathMatchInfo, std::string_view path, Request& request, Response& response)
{
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
        result.emplace(pathMatchInfo.names[loop - 1], path.substr(0, find));

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
    for (auto const& pathMatchInfo: paths)
    {
        if (checkPathMatch(pathMatchInfo, path, request, response)) {
            return true;
        }
    }
    return false;
}
