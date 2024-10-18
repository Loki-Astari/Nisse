#ifndef THORSANVIL_NISSE_PYNTHTTP_REQUEST_H
#define THORSANVIL_NISSE_PYNTHTTP_REQUEST_H

#include "PyntHTTPConfig.h"
#include "Util.h"
#include <istream>

namespace ThorsAnvil::Nisse::PyntHTTP
{

class Request
{
    std::string messageHeader;
    Version     version;
    Method      method;
    URL         url;
    Header      head;
    Header      tail;
    public:
        Request(std::string_view proto, std::istream& stream);
        Version             getVersion()    const   {return version;}
        Method              getMethod()     const   {return method;}
        URL const&          getUrl()        const   {return url;}
        std::string_view    httpRawRequest()const   {return messageHeader;}

        Header const&       headers()       const   {return head;}
        Header const&       trailers()      const   {return tail;}
                                                    // Trailers will return an empty Header() if body has not been read.
                                                    // if (body().eof()) Then trailers have been read.

        std::istream&       body();                 // Can be used to read the stream body.
                                                    // It will auto eof() when no more data is available in the body.
                                                    // Note this stream will auto decode the incoming message body based
                                                    // on the 'content-encoding'
    private:
        std::string_view    readFirstLine(std::istream& stream);
        void                readHeaders(Header& dst, std::istream& stream);

        Version             findVersion(std::string_view pv);
        Method              findMethod(std::string_view method);
        void                buildURL(std::string_view proto, std::string_view path);

        std::string_view    getValue(std::string_view input);
};

}

#endif
