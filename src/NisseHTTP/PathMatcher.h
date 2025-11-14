#ifndef THORSANVIL_NISSE_NISSEHTTP_PATH_MATCHER_H
#define THORSANVIL_NISSE_NISSEHTTP_PATH_MATCHER_H

#include "NisseHTTPConfig.h"
#include "Util.h"
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <functional>

namespace ThorsAnvil::Nisse::HTTP
{

class Request;
class Response;
using Match     = std::map<std::string, std::string>;

class PathMatcher
{
    using Action    = std::function<void(Match const&, Request&, Response&)>;
    using NameList  = std::vector<std::string>;
    using MatchList = std::vector<std::string>;

    struct MatchInfo
    {
        MethodChoice    method;
        MatchList       matchSections;
        NameList        names;
        Action          action;
    };

    std::vector<MatchInfo>  paths;

    public:
        void addPath(MethodChoice method, std::string pathMatch, Action&& action);

        bool findMatch(std::string_view path, Request& request, Response& response);
    private:
        bool checkPathMatch(MatchInfo const& pathMatchInfo, std::string_view path, Request& request, Response& response);
        std::string decode(std::string_view matched);
};

}

#endif
