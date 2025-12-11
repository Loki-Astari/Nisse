#ifndef THORSANVIL_NISSE_NISSEHTTP_HTTP_HANDLER_H
#define THORSANVIL_NISSE_NISSEHTTP_HTTP_HANDLER_H

#include "NisseHTTPConfig.h"
#include "PyntHTTP.h"
#include "PathMatcher.h"
#include <string>
#include <string_view>
#include <functional>
#include <vector>
#include <istream>

namespace ThorsAnvil::Nisse::HTTP
{

class HeaderRequest;

class HTTPHandler: public PyntHTTP
{
    using HTTPAction = std::function<bool(Request& request, Response& response)>;

    PathMatcher             pathMatcher;

    public:
        virtual void       processRequest(Request& request, Response& response) override;

        void addPath(std::string const& path, HTTPAction&& action)  {addPath(All::Method, path, std::move(action));}
        void remPath(std::string const& path)                       {remPath(All::Method, path);}

        void addPath(MethodChoice method, std::string const& path, HTTPAction&& action);
        void remPath(MethodChoice method, std::string const& path);
    private:
        std::string normalize(std::string_view path);

        void addHeaders(RequestVariables& var, HeaderRequest const& headers);
        void addQueryParam(RequestVariables& var, std::string_view query);
        void addPathMatch(RequestVariables& var, Match const& matches);
        void addFormVariables(RequestVariables& var, std::istream& stream);
};

}

#endif
