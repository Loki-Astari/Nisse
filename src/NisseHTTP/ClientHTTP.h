#ifndef THORSANVIL_NISSE_HTTP_CLIENT_HTTP_H
#define THORSANVIL_NISSE_HTTP_CLIENT_HTTP_H

#include "NisseHTTPConfig.h"
#include "HeaderRequest.h"
#include "StreamInput.h"
#include "StreamOutput.h"
#include "ThorsLogging/ThorsLogging.h"

#include "Util.h"

#include "ThorsSocket/SocketStream.h"
#include "ThorsSocket/SocketUtil.h"

#include "ThorSerialize/JsonThor.h"
#include "ThorSerialize/Traits.h"
#include "ThorSerialize/SerUtil.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

namespace ThorsAnvil::Nisse::HTTP
{
    struct ClientRequest
    {
        std::string             path;
        HeaderRequest const&    headers = {};
    };

    class ClientHTTPResponse
    {
            Version                     version;
            int                         status;
            std::string                 message;
            HeaderRequest               headers;
            mutable StreamInput         input;
            bool                        valid;
        public:
            ~ClientHTTPResponse()
            {}
            ClientHTTPResponse(std::iostream& stream)
            {
                valid = readFirstLine(stream) && buildStream(stream);
            }
            ClientHTTPResponse(ClientHTTPResponse const&)                       = delete;
            ClientHTTPResponse& operator=(ClientHTTPResponse const&)            = delete;
            ClientHTTPResponse(ClientHTTPResponse&& move) noexcept              = delete;
            ClientHTTPResponse& operator=(ClientHTTPResponse&& move) noexcept   = delete;

            Version                 getVersion()    const   {return version;}
            int                     getStatus()     const   {return status;}
            std::string const&      getMessage()    const   {return message;}
            HeaderRequest const&    getHeader()     const   {return headers;}
            StreamInput&            body()          const   {return input;}

        private:
            bool readFirstLine(std::iostream& stream);
            bool buildStream(std::iostream& stream);

            friend class ClientHTTPBase;
            bool isValid() const {return valid;}
    };

    class ClientHTTPBase
    {
        std::iostream&          stream;
        std::string_view        host;
        Version                 version;
        std::function<void()>   close;

        public:
            ClientHTTPBase(std::iostream& stream, std::string_view host, Version version = Version::HTTP1_1, std::function<void()>&& close = [](){})
                : stream{stream}
                , host{host}
                , version{version}
                , close{std::move(close)}
            {}

            void get(ClientRequest const& request)                      {send(Method::GET, request, 0, [](std::ostream&){});}
            template<typename T>
            void put(ClientRequest const& request, T const& data)       {send(Method::PUT, request, ThorsAnvil::Serialize::jsonStreanSize(data), [&data](std::ostream& output){output << ThorsAnvil::Serialize::jsonExporter(data);});}
            template<typename T>
            void post(ClientRequest const& request, T const& data)      {send(Method::POST, request, ThorsAnvil::Serialize::jsonStreanSize(data), [&data](std::ostream& output){output << ThorsAnvil::Serialize::jsonExporter(data);});}

            template<typename A>
            void send(Method method, ClientRequest const& request, BodyEncoding encoding, A&& action)
            {
                // Send to the server a correctly encoded HTTP request.
                // With the minumum headers.
                stream << method << " " << request.path << " " << version << "\r\n"
                       << "host: " << host << "\r\n"
                       << encoding;

                // Add the user requested header.
                for (auto const& header: request.headers) {
                    for (auto const& value: header.second) {
                        stream << header.first << ": " << value << "\r\n";
                    }
                }

                // Empty line marking end of headers.
                stream << "\r\n";

                /*
                 * Create a stream that knows how to enforce the encoding specified.
                 *
                 * If you specified an exact size (via integer) then the stream will set the bad bit if you send
                 * more data than the allowed value. But if you don't send enough will fill the output stream in
                 * the destructor.
                 *
                 * If the encoding is chunked it will buffer internally until you hit the buffer size or manually flush;
                 * at which point it will send the hex encoded size followed by the data in the buffer.
                 * The destructor will then make sure then stream is correctly terminated with an empty block.
                 *
                 * The same pattern for other encodings. The stream will automatically apply the encoding.
                 * And the destructor of `output` will make sure the stream is correctly terminated and flushed.
                 */
                StreamOutput    output(stream, encoding);
                std::forward<A>(action)(output);
            }

            template<typename A>
            void processResp(A&& action)
            {
                // Reads the status line and header information from the stream.
                // Internally it will create a stream object that decodes the input based on the headers).
                // So your code can read directly from the input.
                ClientHTTPResponse  response{stream};
                if (!response.isValid()) {
                    close();
                }
                std::forward<A>(action)(response);
            }
    };
    class ClientHTTP: public ClientHTTPBase
    {
        ThorsAnvil::ThorsSocket::SocketStream   stream;

        public:
            ClientHTTP(ThorsAnvil::ThorsSocket::SSocketInfo const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, info.host, version, [&](){stream.close();}}
                , stream{info}
            {}
            ClientHTTP(ThorsAnvil::ThorsSocket::SSocketService const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, info.host, version, [&](){stream.close();}}
                , stream{info}
            {}
            ClientHTTP(ThorsAnvil::ThorsSocket::SocketInfo const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, info.host, version, [&](){stream.close();}}
                , stream{info}
            {}
    };
}

#endif
