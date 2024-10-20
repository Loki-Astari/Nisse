#ifndef THORSANVIL_NISSE_PYNTHTTP_RESPONSE_H
#define THORSANVIL_NISSE_PYNTHTTP_RESPONSE_H

#include "PyntHTTPConfig.h"
#include "Util.h"
#include "HeaderResponse.h"
#include "StreamOutput.h"
#include <set>
#include <ostream>
#include <functional>

namespace ThorsAnvil::Nisse::PyntHTTP
{

struct StatusCode
{
    int                 code;
    std::string_view    message;

    friend std::ostream& operator<<(std::ostream& stream, StatusCode const& statusCode)
    {
        return stream << statusCode.code << " " << statusCode.message << "\r\n";
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
    bool            bodySent;

    std::ostream&   baseStream;
    StreamOutput    stream;

    public:
        Response(std::ostream& stream, Version version, int code = 200);
        ~Response();
        void                setStatus(int code);

        std::ostream&       addHeaders(HeaderResponse const& headers, Encoding type);
        std::ostream&       addHeaders(HeaderResponse const& headers, std::size_t length);
    private:
        std::ostream&       addHeaders(HeaderResponse const& headers, StreamBufOutput&& buffer, std::string_view extraHeader);
};

}

#endif
