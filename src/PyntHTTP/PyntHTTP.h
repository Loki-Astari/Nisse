#ifndef THORSANVIL_NISSE_PYNT_HTTP_H
#define THORSANVIL_NISSE_PYNT_HTTP_H

/*
 * An HTTP implementation of Pynt
 * Version 1:
 */

#include "PyntHTTPConfig.h"
#include "NisseServer/Pynt.h"
#include <string_view>

namespace ThorsAnvil::Nisse
{

enum class Version {HTTP1_0, HTTTP1_1, HTTP2, HTTP3, Uknown};
enum class Method  {GET, HEAD, OPTIONS, TRACE, PUT, DELETE, POST, PATCH, CONNECT, Other};

class URL
{
    public:
        std::string_view        href();     // 'http://user:pass@localhost:53/status?name=ryan#234'
        std::string_view        origin();   // 'http://localhost:53'
        std::string_view        protocol(); // 'http:'
        std::string_view        username(); // 'user'
        std::string_view        password(); // 'pass'
        std::string_view        host();     // 'localhost:53',
        std::string_view        hostname(); // 'localhost',
        std::string_view        port();     // '53'
        std::string_view        pathname(); // '/status',
        std::string_view        search();   // '?name=ryan',
        std::string_view        hash();     // '#234'

        std::string_view        param(std::string_view param);  // parm('name') => 'ryan'
};

class Header
{
    using CIterator = std::pair<std::string, std::vector<std::string>>;
    using const_iterator = CIterator;

    public:
        std::vector<std::string>&   getHeader(std::string_view header);
        bool                        hasHeader(std::string_view header);
        CIterator                   begin() const;
        CIterator                   end()   const;
};

class Request
{
    Version             httpVersion();
    Method              method();
    URL const&          url();
    std::string_view    httpRawRequest();   // GET /index.html HTTP/1.1

    Header const&       headers();
    Header const&       trailers();         // Trailers will return an empty Header() if body has not been read.
                                            // if (body().eof()) Then trailers have been read.

    std::istream&       body();             // Can be used to read the stream body.
                                            // It will auto eof() when no more data is available in the body.
                                            // Note this stream will auto decode the incoming message body based
                                            // on the 'content-encoding'
};

class Response
{
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

class PyntHTTP: public Pynt
{
    public:
        virtual PyntResult handleRequest(std::iostream& stream) override;
        virtual PyntResult processRequest(Request const& request, Response& response) = 0;
};

}

#endif
