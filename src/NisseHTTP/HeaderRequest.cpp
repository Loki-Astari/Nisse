#include "HeaderRequest.h"
#include <set>
#include <sstream>

using namespace ThorsAnvil::Nisse::HTTP;

std::vector<std::string> const& HeaderRequest::getHeader(std::string const& header) const
{
    static const std::vector<std::string>   empty;
    auto find = headers.find(header);
    if (find == headers.end()) {
        return empty;
    }
    return find->second;
}

void HeaderRequest::add(std::string_view header, std::string_view value)
{
    header = getValue(header);
    std::string key(header.size(), ' ');
    std::transform(std::begin(header), std::end(header), std::begin(key), [](unsigned char c){ return std::tolower(c);});

    if (key == "set-cookie")
    {
        // SPECIAL HANDLING for Cookies.
        // TODO
        return;
    }

    auto& headerValue   = headers[key];
    if (dedupHeader(key))
    {
        if (headerValue.size() == 1) {
            return;
        }
        headerValue.emplace_back(getValue(value));
        return;
    }
    if (splitOnComma(key))
    {
        std::stringstream ss{std::string{value}};
        std::string       splitValue;

        while (std::getline(ss, splitValue, ','))
        {
            headerValue.emplace_back(getValue(splitValue));
        }
    }
    else
    {
        headerValue.emplace_back(getValue(value));
    }
}

std::string_view HeaderRequest::getValue(std::string_view input)
{
    // Remove leading and trailing white space.
    input.remove_prefix(std::min(input.size(), input.find_first_not_of(" \r\t\v")));
    input.remove_suffix(input.size() - std::min(input.size(), input.find_last_not_of(" \r\t\v")) - 1);
    return input;
}

bool HeaderRequest::dedupHeader(std::string_view header)
{
    // See RFC 9110 Section 5.3 for more information.
    static const std::set<std::string_view> singleValueHeaders = {"age", "authorization", "content-length", "content-type", "etag", "expires", "from", "host", "if-modified-since", "if-unmodified-since", "last-modified", "location", "max-forwards", "proxy-authorization", "referer", "retry-after", "server", "user-agent"};
    return singleValueHeaders.find(header) != singleValueHeaders.end();
}

bool HeaderRequest::splitOnComma(std::string_view header)
{
    static const std::set<std::string_view> nosplit = {"accept-datetime", "access-control-request-method", "access-control-request-header", "content-md5", "cookie", "date", "expect", "if-match", "if-none-match", "if-range"};
    return nosplit.find(header) == nosplit.end();
}

void HeaderRequest::print(std::ostream& stream) const
{
    for (auto const& header: headers)
    {
        stream << header.first << ": ";
        std::string     sep = "";
        for (auto const& val: header.second)
        {
            stream << sep << val;
            sep = ", ";
        }
        stream << "\r\n";
    }
}
