#ifndef THORSANVIL_NISSE_NISSEHTTP_HTTP_HANDLER_H
#define THORSANVIL_NISSE_NISSEHTTP_HTTP_HANDLER_H

#include "NisseHTTPConfig.h"
#include "PyntHTTP.h"
#include "PathMatcher.h"
#include "Util.h"
#include <string_view>

namespace ThorsAnvil::Nisse::HTTP
{

class HeaderRequest;

class HTTPHandler: public PyntHTTP
{
    using HTTPAction = std::function<void(Request& request, Response& response)>;

    PathMatcher             pathMatcher;
    std::vector<HTTPAction> actions;
    public:
        virtual void       processRequest(Request& request, Response& response) override;

        void addPath(std::string const& path, HTTPAction&& action);
        void addPath(MethodChoice method, std::string const& path, HTTPAction&& action);
    private:
        std::string normalize(std::string_view path);

        void addHeaders(RequestVariables& var, HeaderRequest const& headers);
        void addQueryParam(RequestVariables& var, std::string_view query);
        void addPathMatch(RequestVariables& var, Match const& matches);
        void addFormVariables(RequestVariables& var, std::istream& stream);
};

}

#endif
