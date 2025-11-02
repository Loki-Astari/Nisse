#ifndef THORSANVIL_NISSE_NISSEHTTP_UTIL_H
#define THORSANVIL_NISSE_NISSEHTTP_UTIL_H

#include "NisseHTTPConfig.h"
#include <set>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>

namespace ThorsAnvil::Nisse::HTTP
{

enum class Version {HTTP1_0, HTTP1_1, HTTP2, HTTP3, Unknown};
enum class Method  {GET, HEAD, OPTIONS, TRACE, PUT, DELETE, POST, PATCH, CONNECT, Other};
enum class All     {Method};
enum class Encoding{Chunked};

using MethodChoice = std::variant<Method, All>;
using BodyEncoding = std::variant<std::size_t, std::streamsize, Encoding>;


std::ostream& operator<<(std::ostream&, Version const& v);
std::istream& operator>>(std::istream&, Version& v);
std::ostream& operator<<(std::ostream&, BodyEncoding const& bodyEncoding);
std::ostream& operator<<(std::ostream&, Encoding const& e);
std::ostream& operator<<(std::ostream&, Method const& method);

using RequestVariables = std::map<std::string, std::string>;

static auto ichar_equals = [](char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
};

}

#endif
