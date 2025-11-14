#ifndef THORSANVIL_NISSE_NISSEHTTP_RESPONSE_H
#define THORSANVIL_NISSE_NISSEHTTP_RESPONSE_H

#include "NisseHTTPConfig.h"
#include "Util.h"
#include "HeaderResponse.h"
#include "HeaderPassThrough.h"
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

    friend std::ostream& operator<<(std::ostream& stream, StatusCode const& statusCode) {statusCode.print(stream);return stream;}
    void print(std::ostream& stream) const;
};

class StandardStatusCodeMap
{
    using StatusCodeMap = std::set<StatusCode, std::function<bool(StatusCode const& lhs, StatusCode const& rhs)>>;

    static StatusCodeMap const standardCodes;
    public:
        StatusCode const& operator[](int code);
};

using Header = std::variant<std::reference_wrapper<HeaderResponse const>, std::reference_wrapper<HeaderPassThrough const>>;

class Response
{
    Version         version;
    StatusCode      statusCode;
    bool            headerSent;

    std::ostream&   baseStream;
    StreamOutput    stream;

    void sendHeaderIfNotSent();

    public:
        Response(std::ostream& stream, Version version, int code = 200);
        ~Response();

        // Read a response from another server.
        friend std::istream& operator>>(std::istream& stream, Response& response)  {response.read(stream);return stream;}
        void read(std::istream& stream);


        // Build up the response message.
        void          setStatus(int code);
        void          addHeaders(Header const& headers);
        std::ostream& body(BodyEncoding encoding);

        // Simplifications to handling common cases.
        void          error(int code, std::string_view errorMessage);   // Call(s) setStatus() adds A single header with the Error Message.

        StatusCode const& getCode() const {return statusCode;}
};

}

#endif
