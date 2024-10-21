#include "PathMatcher.h"

using namespace ThorsAnvil::Nisse::NisseHTTP;

void PathMatcher::addPath(std::string pathMatch, Action&& action)
{
    // Variables to be built.
    std::string     expr;       // Convert pathMatch into a regular expression.
    NameList        names;      // Extract list of names from pathMatch.

    // Search variables
    std::smatch     searchMatch;
    std::regex      pathNameExpr{"\\{[^}]*\\}"};

    while (std::regex_search(pathMatch, searchMatch, pathNameExpr))
    {
        expr += pathMatch.substr(0, searchMatch.position());
        expr += "([^/]*)";

        std::string match = searchMatch.str();
        names.emplace_back(match.substr(1, match.size() - 2));

        pathMatch = searchMatch.suffix().str();
    }
    expr += pathMatch;

    // Add the path information to the list.
    paths.emplace_back(std::regex{expr}, std::move(names), std::move(action));
}

void PathMatcher::findMatch(std::string const& path, Request& request, Response& response)
{
    for (auto const& pathMatchInfo: paths)
    {
        std::smatch     match{};
        if (std::regex_match(path, match, pathMatchInfo.test))
        {
            Match   result;
            for (std::size_t loop = 0; loop < pathMatchInfo.names.size(); ++loop)
            {
                result.insert({pathMatchInfo.names[loop], match[loop+1].str()});
            }
            pathMatchInfo.action(result, request, response);
            break;
        }
    }
}
