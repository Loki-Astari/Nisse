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

class RequestVariables
{
    std::map<std::string, std::string>      store;
    public:
        std::size_t size()                  const {return store.size();}
        bool exists(std::string const& key) const {return store.find(key) != std::end(store);}
        std::string const& operator[](std::string const& key) const
        {
            static std::string empty;
            auto find = store.find(key);
            if (find == std::end(store)) {
                return empty;
            }
            return find->second;
        }
        void insert_or_assign(std::string const& key, std::string const& value)
        {
            store.insert_or_assign(key, value);
        }
        void insert_or_assign(std::string const& key, std::string&& value)
        {
            store.insert_or_assign(key, std::move(value));
        }
        void insert_or_assign(std::string&& key, std::string&& value)
        {
            store.insert_or_assign(std::move(key), std::move(value));
        }
};

static auto ichar_equals = [](char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
};

}

#endif
