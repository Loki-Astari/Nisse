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

namespace
{
    static constexpr int asciiHexToInt[] =
{
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
};

    bool isHex(unsigned char c)      {return asciiHexToInt[c] >= 0;}
    int  decodeHex(unsigned char c)  {return asciiHexToInt[c];}
}
std::string decode(std::string_view view)
{
    std::string result;
    for (std::size_t loop = 0; loop < std::size(view); ++loop) {
        if (view[loop] != '%') {
            result += view[loop];
            continue;
        }
        if (loop + 2 >= std::size(view)) {
            result += view[loop];
            continue;
        }
        if (!isHex(view[loop + 1]) || !isHex(view[loop + 2])) {
            result += view[loop];
            continue;
        }
        result += static_cast<char>(decodeHex(view[loop + 1]) * 16 + decodeHex(view[loop + 2]));
        loop += 2;
    }
    if (std::string_view{std::begin(result), std::begin(result) + 3} == "\xEF\xBB\xBF") {
        result.erase(0, 3);
    }
    return result;
}

void HTTPHandler::addFormVariables(RequestVariables& var, std::istream& stream)
{
    std::string     line;
    while (std::getline(stream, line, '&')) {
        auto findEqual = std::min(std::size(line), line.find('='));
        auto valueStart = findEqual + (findEqual == std::size(line) ? 0 : 1);
        std::string nameView{std::begin(line), std::begin(line) + findEqual};
        std::string valueView{std::begin(line) + valueStart, std::end(line)};

        std::replace(std::begin(nameView), std::end(nameView), '+', ' ');
        std::replace(std::begin(valueView), std::end(valueView), '+', ' ');

        std::string name = decode(nameView);
        std::string value = decode(valueView);

        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addFormVariables", name, " => ", value);
        var.insert_or_assign(std::move(name), std::move(value));
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

        if (var["content-type"] == "application/x-www-form-urlencoded") {
            addFormVariables(var, request.body());
        }

        actions[actionId](request, response);
        ThorsLogDebug("ThorsAnvil::Nisse::HTTP::HTTPHandler", "addPath>Lambda<", "Calling User Function DONE");
    });
}
