#ifndef THORSANVIL_NISSE_HTTP_CLIENT_HTTP_H
#define THORSANVIL_NISSE_HTTP_CLIENT_HTTP_H

#include "NisseHTTPConfig.h"
#include "HeaderRequest.h"
#include "StreamInput.h"
#include "StreamOutput.h"

#include "ThorsLogging/ThorsLogging.h"
#include "ThorsSocket/SocketStream.h"
#include "ThorSerialize/JsonThor.h"

#include <string>
#include <iostream>
#include <string_view>
#include <functional>
#include <utility>

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

            void get(ClientRequest const& request)                      const {send(Method::GET, request, 0, [](StreamOutput&){});}
            template<typename T>
            void put(ClientRequest const& request, T const& data)       const {send(Method::PUT, request, ThorsAnvil::Serialize::jsonStreanSize(data), [&data](std::ostream& output){output << ThorsAnvil::Serialize::jsonExporter(data);});}
            template<typename T>
            void post(ClientRequest const& request, T const& data)      const {send(Method::POST, request, ThorsAnvil::Serialize::jsonStreanSize(data), [&data](std::ostream& output){output << ThorsAnvil::Serialize::jsonExporter(data);});}

            void send(Method method, ClientRequest const& request, BodyEncoding encoding, std::function<void(StreamOutput& action)>&& action) const;
            void processResp(std::function<void(ClientHTTPResponse const&)>&& action) const;
            std::string getRespAsString() const;
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
