#ifndef THORSANVIL_NISSE_PYNTHTTP_HEADER_RESPONSE_H
#define THORSANVIL_NISSE_PYNTHTTP_HEADER_RESPONSE_H

#include "PyntHTTPConfig.h"
#include <string_view>
#include <string>
#include <map>
#include <iostream>

namespace ThorsAnvil::Nisse::PyntHTTP
{

inline bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

class HeaderResponse
{
    std::map<std::string, std::string>  headers;
    public:
        bool    empty() const;
        void    add(std::string_view header, std::string_view value);

    friend std::ostream& operator<<(std::ostream& stream, HeaderResponse const& headersBlock)
    {
        for (auto header: headersBlock.headers)
        {
            if (std::ranges::equal(std::string_view(header.first), std::string_view("content-length"), ichar_equals)) {
                continue;
            }
            if (std::ranges::equal(std::string_view(header.first), std::string_view("transfer-encoding"), ichar_equals)) {
                continue;
            }
            stream << header.first << ": " << header.second << "\r\n";
        }
        return stream;
    }
};

}

#endif
