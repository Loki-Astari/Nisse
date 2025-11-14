#include "HTTPHandler.h"
#include "Request.h"
#include "Response.h"
#include <regex>

using namespace ThorsAnvil::Nisse::HTTP;

std::string HTTPHandler::normalize(std::string_view path)
{
    return std::string(path);
}

void HTTPHandler::processRequest(Request& request, Response& response)
{
    std::string path = normalize(request.getUrl().pathname());

    if (!pathMatcher.findMatch(path, request, response)) {
        response.setStatus(404);
    }
}

void HTTPHandler::addHeaders(RequestVariables& var, HeaderRequest const& headers)
{
    for (auto const& head: headers) {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addHeaders", head.first, " => ", head.second.back());
        var.insert_or_assign(head.first, head.second.back());
    }
}

void HTTPHandler::addQueryParam(RequestVariables& var, std::string_view query)
{
    // Add URL parameters to the variables object.
    if (query.size() == 0) {
        return;
    }

    // Remove the first ? or &
    query.remove_prefix(1);

    std::smatch     queryParamMatch;
    std::regex      queryParamExpr{"([^=]*)=([^&]*)&"};
    std::string     queryStr(query);
    queryStr += "&";

    while (std::regex_search(queryStr, queryParamMatch, queryParamExpr))
    {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addQueryParam", queryParamMatch[1].str(), " => ", queryParamMatch[2].str());
        var.insert_or_assign(queryParamMatch[1].str(), queryParamMatch[2].str());
        queryStr = queryParamMatch.suffix().str();
    }
}

void HTTPHandler::addPathMatch(RequestVariables& var, Match const& matches)
{
    for (auto const& match: matches) {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addPathMatch", match.first, " => ", match.second);
        var.insert_or_assign(match.first, match.second);
    }
}

void HTTPHandler::addPath(MethodChoice method, std::string const& path, HTTPAction&& action)
{
    actions.emplace_back(std::move(action));

    pathMatcher.addPath(method, path, [&, actionId = actions.size() - 1](Match const& matches, Request& request, Response& response)
    {
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addPath>Lambda<", "Calling User Function");
        // Get the variable object
        RequestVariables&   var     = request.variables();

        addHeaders(var, request.headers());
        addQueryParam(var,  request.getUrl().query());
        addPathMatch(var, matches);

        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addPath>Lambda<", "Calling User Function NOW");
        actions[actionId](request, response);
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addPath>Lambda<", "Calling User Function DONE");
    });
}
