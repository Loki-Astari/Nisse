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
    using Action    = std::function<bool(Match const&, Request&, Response&)>;
    using NameList  = std::vector<std::string>;
    using MatchList = std::vector<std::string>;

    struct MatchBase
    {
        MethodChoice    method;
        MatchList       matchSections;
        NameList        names;

        bool operator==(MatchBase const& rhs) const
        {
            return std::tie(method, matchSections, names) == std::tie(rhs.method, rhs.matchSections, rhs.names);
        }
    };
    struct MatchInfo: public MatchBase
    {
        MatchInfo(MethodChoice method, MatchList matchSections, NameList names, Action action)
            : MatchBase(std::move(method), std::move(matchSections), std::move(names))
            , action(std::move(action))
        {}
        Action          action;
    };

    std::vector<MatchInfo>  paths;

    public:
        void addPath(MethodChoice method, std::string pathMatch, Action&& action);
        void remPath(MethodChoice method, std::string pathMatch);

        bool findMatch(std::string_view path, Request& request, Response& response);
    private:
        MatchBase   buildMatchInfo(MethodChoice method, std::string pathMatch);
        bool        checkPathMatch(MatchInfo const& pathMatchInfo, std::string_view path, Request& request, Response& response);
        std::string decode(std::string_view matched);
};

}

#endif
