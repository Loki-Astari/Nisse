#ifndef THORSANVIL_NISSE_NISSEHTTP_PATH_MATCHER_H
#define THORSANVIL_NISSE_NISSEHTTP_PATH_MATCHER_H

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <regex>

namespace ThorsAnvil::Nisse::NisseHTTP
{

class Request;
class Response;

using Match     = std::map<std::string, std::string>;
using Action    = std::function<void(Match const&, Request&, Response&)>;
using NameList  = std::vector<std::string>;

class PathMatcher
{
    struct MatchInfo
    {
        std::regex  test;
        NameList    names;
        Action      action;
    };

    std::vector<MatchInfo>  paths;

    public:
        void addPath(std::string pathMatch, Action&& action);

        bool findMatch(std::string const& path, Request& request, Response& response);
};

}

#endif
