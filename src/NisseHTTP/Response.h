#ifndef THORSANVIL_NISSE_NISSEHTTP_RESPONSE_H
#define THORSANVIL_NISSE_NISSEHTTP_RESPONSE_H

#include "NisseHTTPConfig.h"
#include "Util.h"
#include "HeaderResponse.h"
#include "StreamOutput.h"
#include <set>
#include <ostream>
#include <functional>

namespace ThorsAnvil::Nisse::HTTP
{

struct StatusCode
{
    int                 code;
    std::string_view    message;

    friend std::ostream& operator<<(std::ostream& stream, StatusCode const& statusCode)
    {
        return stream << statusCode.code << " " << statusCode.message;
    }
};

class StandardStatusCodeMap
{
    using StatusCodeMap = std::set<StatusCode, std::function<bool(StatusCode const& lhs, StatusCode const& rhs)>>;

    static StatusCodeMap const standardCodes;
    public:
        StatusCode const& operator[](int code);
};


class Response
{
    Version         version;
    StatusCode      statusCode;
    bool            headerSent;

    std::ostream&   baseStream;
    StreamOutput    stream;

    public:
        Response(std::ostream& stream, Version version, int code = 200);
        ~Response();
        void                setStatus(int code);

        friend std::istream& operator>>(std::istream& stream, Response& response)  {response.read(stream);return stream;}
        void read(std::istream& stream);

        std::ostream&       addHeaders(HeaderResponse const& headers, BodyEncoding encoding);
};

}

#endif
