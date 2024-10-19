#ifndef THORSANVIL_NISSE_PYNTHTTP_RESPONSE_H
#define THORSANVIL_NISSE_PYNTHTTP_RESPONSE_H

#include "PyntHTTPConfig.h"
#include "Util.h"
#include "HeaderResponse.h"
#include <set>
#include <ostream>
#include <functional>

namespace ThorsAnvil::Nisse::PyntHTTP
{

struct StatusCode
{
    int                 code;
    std::string_view    message;
};

class StandardStatusCodeMap
{
    using StatusCodeMap = std::set<StatusCode, std::function<bool(StatusCode const& lhs, StatusCode const& rhs)>>;

    static StatusCodeMap const standardCodes;
    public:
        StatusCode const& operator[](int code);
};
extern StandardStatusCodeMap standardCodes;


class Response
{
    Version     version;
    StatusCode  statusCode;
    bool        headerSent;
    bool        bodySent;
    public:
        Response(std::ostream& stream, Version version, StatusCode const& code = standardCodes[200]);
        ~Response();
        void                setStatus(StatusCode const& code);

        std::ostream        addHeaders(HeaderResponse const& headers);
        void                done();
#if 0
                                                                            // Can be modified anytime up to the headers being sent.
        //Header&             headers();          // Headers will be locked after they are sent. By default this happens when data is sent to the body stream.
        void                done();             // Manually indicate complete. Done automatically by destructor.
                                                // But processes can use this to flush stream immediately if desired.

        std::ostream&       body();             // Format of body is determined by the 'content-encoding/content-length' header fields.
                                                // User will simply write to the stream the stream will automatically encode the data.
                                                // Note: Headers will be locked after you start writing to the body stream and thus can
                                                //       once writing commences the content-encoding/content-length can not be changed.

        bool                isHeadersSent() const;
        bool                isDone()        const;
    private:
        void                sendHeaders();
#endif
};

}

#endif
