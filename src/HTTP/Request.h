#ifndef THORSANVIL_NISSE_HTTP_REQUEST_H
#define THORSANVIL_NISSE_HTTP_REQUEST_H

#include "HTTPConfig.h"
#include "URL.h"
#include "HeaderRequest.h"
#include "HeaderResponse.h"
#include "StreamInput.h"
#include <istream>

namespace ThorsAnvil::Nisse::HTTP
{

class Request
{
    std::string     messageHeader;
    Version         version;
    Method          method;
    URL             url;
    HeaderRequest   head;
    HeaderRequest   tail;
    HeaderResponse  failResponse;

    StreamInput input;

    std::unique_ptr<std::streambuf> streamBuf;
    public:
        Request(std::string_view proto, std::istream& stream);
        Version                 getVersion()    const   {return version;}
        Method                  getMethod()     const   {return method;}
        URL const&              getUrl()        const   {return url;}
        std::string_view        httpRawRequest()const   {return messageHeader;}

        HeaderRequest const&    headers()       const   {return head;}
        HeaderRequest const&    trailers()      const   {return tail;}
        HeaderResponse const&   failHeader()    const   {return failResponse;}
        bool                    isValidRequest()const   {return failResponse.empty();}
                                                    // Trailers will return an empty HeaderRequest() if body has not been read.
                                                    // if (body().eof()) Then trailers have been read.

        std::istream&           body();             // Can be used to read the stream body.
                                                    // It will auto eof() when no more data is available in the body.
                                                    // Note this stream will auto decode the incoming message body based
                                                    // on the 'content-encoding'

    private:
        std::string_view        readFirstLine(std::istream& stream);
        bool                    readHeaders(HeaderRequest& dst, std::istream& stream);

        Version                 findVersion(std::string_view pv);
        Method                  findMethod(std::string_view method);
        bool                    buildURL(std::string_view proto, std::string_view path);
        bool                    buildStream(std::istream& stream);

        std::string_view        getValue(std::string_view input);
};

}

#endif
