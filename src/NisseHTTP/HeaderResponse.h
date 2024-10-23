#ifndef THORSANVIL_NISSE_NISSEHTTP_HEADER_RESPONSE_H
#define THORSANVIL_NISSE_NISSEHTTP_HEADER_RESPONSE_H

#include "NisseHTTPConfig.h"
#include <string_view>
#include <string>
#include <map>
#include <iostream>

namespace ThorsAnvil::Nisse::HTTP
{

class HeaderResponse
{
    std::map<std::string, std::string>  headers;
    public:
        bool    empty() const;
        void    add(std::string_view header, std::string_view value);

    friend std::ostream& operator<<(std::ostream& stream, HeaderResponse const& headersBlock)   {headersBlock.print(stream);return stream;}

    void print(std::ostream& stream) const;
};

}

#endif
