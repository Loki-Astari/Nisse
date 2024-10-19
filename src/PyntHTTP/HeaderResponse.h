#ifndef THORSANVIL_NISSE_PYNTHTTP_HEADER_RESPONSE_H
#define THORSANVIL_NISSE_PYNTHTTP_HEADER_RESPONSE_H

#include "PyntHTTPConfig.h"
#include <string_view>
#include <string>
#include <map>

namespace ThorsAnvil::Nisse::PyntHTTP
{

class HeaderResponse
{
    std::map<std::string, std::string>  headers;
    public:
        bool    empty() const;
        void    add(std::string_view header, std::string_view value);
};

}

#endif
