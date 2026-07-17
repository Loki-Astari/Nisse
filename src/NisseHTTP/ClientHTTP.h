#ifndef THORSANVIL_NISSE_HTTP_CLIENT_HTTP_H
#define THORSANVIL_NISSE_HTTP_CLIENT_HTTP_H

#include "NisseHTTPConfig.h"
#include "HeaderRequest.h"
#include "StreamInput.h"
#include "Util.h"

#include "ThorsSocket/SocketStream.h"
#include "ThorsSocket/SocketUtil.h"

#include "ThorSerialize/JsonThor.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>

namespace ThorsAnvil::Nisse::HTTP
{
    class ClientHTTPResponse
    {
            ThorsAnvil::ThorsSocket::SocketStream*      stream;
            Version         version;
            int             status;
            std::string     message;
            HeaderRequest   headers;

            mutable StreamInput             input;
            // std::unique_ptr<std::streambuf> streamBuf;
        public:
            ~ClientHTTPResponse()
            {}
            ClientHTTPResponse(ThorsAnvil::ThorsSocket::SocketStream& stream)
                : stream(&stream)
            {
                readFirstLine();
                buildStream();
            }
            ClientHTTPResponse(ClientHTTPResponse const&)               = delete;
            ClientHTTPResponse& operator=(ClientHTTPResponse const&)    = delete;
            ClientHTTPResponse(ClientHTTPResponse&& move) noexcept
                : stream(std::exchange(move.stream, nullptr))
                , version(std::exchange(move.version, Version::Unknown))
                , status(std::exchange(move.status, 0))
                , message(std::exchange(move.message, ""))
                , headers(std::exchange(move.headers, {}))
            {}
            ClientHTTPResponse& operator=(ClientHTTPResponse&& move) noexcept
            {
                move.swap(*this);
                return *this;
            }
            void swap(ClientHTTPResponse& other) noexcept
            {
                using std::swap;
                swap(stream,    other.stream);
                swap(version,   other.version);
                swap(status,    other.status);
                swap(message,   other.message);
                swap(headers,   other.headers);
            }

            std::istream& body()    {return input;}
            std::ostream& output()  {return *stream;}

        private:
            void readFirstLine();
            bool buildStream();
    };

    class ClientHTTP
    {
        ThorsAnvil::ThorsSocket::SocketInfo     init;
        ThorsAnvil::ThorsSocket::SocketStream   stream;
        Version                                 version;

        public:
            ClientHTTP(ThorsAnvil::ThorsSocket::SocketInfo&& info, Version version = Version::HTTP2)
                : init{std::move(info)}
                , stream{init}
                , version{version}
            {}

            ClientHTTPResponse get(std::string_view path, HeaderRequest const& headers = {})               {return send(Method::GET, path, headers, 0);}
            template<typename T>
            ClientHTTPResponse put(std::string_view path, HeaderRequest const& headers, T const& data)     {return send(Method::PUT, path, headers, data);}
            template<typename T>
            ClientHTTPResponse post(std::string_view path, HeaderRequest const& headers, T const& data)    {return send(Method::POST, path, headers, data);}


            ClientHTTPResponse send(Method method, std::string_view path, HeaderRequest const& headers = {}, std::size_t size = 0);
            ClientHTTPResponse send(Method method, std::string_view path, HeaderRequest const& headers = {}, Encoding encoding = Encoding::Chunked);

            template<typename T>
            requires (!std::is_integral_v<std::remove_cvref_t<T>>)
            ClientHTTPResponse send(Method method, std::string_view path, HeaderRequest const& headers, T const& data)
            {
                sendHeader(method, path, headers);
                stream << "content-length: "
                       << ThorsAnvil::Serialize::jsonStreanSize(data) << "\r\n\r\n"
                       << ThorsAnvil::Serialize::jsonExporter(data);

                return ClientHTTPResponse{stream};
            }

        private:
            void sendHeader(Method method, std::string_view path, HeaderRequest const& headers);
    };
}

#endif
