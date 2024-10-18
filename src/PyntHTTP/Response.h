#ifndef THORSANVIL_NISSE_PYNTHTTP_RESPONSE_H
#define THORSANVIL_NISSE_PYNTHTTP_RESPONSE_H

#include "PyntHTTPConfig.h"
#include "Util.h"
#include <ostream>

namespace ThorsAnvil::Nisse::PyntHTTP
{

class Response
{
    public:
        Response(std::ostream&) {}
        void                setStatus(int code, std::string_view message);  // If not set default 200 OK
                                                                            // Can be modified anytime up to the headers being sent.
        Header&             headers();          // Headers will be locked after they are sent. By default this happens when data is sent to the body stream.
        Header&             trailers();
        void                done();             // Manually indicate complete. Done automatically by destructor.
                                                // But processes can use this to flush stream immediately if desired.

        std::ostream&       body();             // Format of body is determined by the 'content-encoding/content-length' header fields.
                                                // User will simply write to the stream the stream will automatically encode the data.
                                                // Note: Headers will be locked after you start writing to the body stream and thus can
                                                //       once writing commences the content-encoding/content-length can not be changed.

        bool                isHeadersSent();
        bool                isDone();
};

}

#endif
