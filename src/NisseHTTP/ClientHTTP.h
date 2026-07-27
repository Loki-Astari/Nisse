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
        std::iostream&                      stream;
        Version                             version;
        std::function<void()>               close;
        std::function<std::string_view()>   host;
        std::function<bool()>               reset;
        mutable bool                        closed;

        public:
            ClientHTTPBase(std::iostream& stream, Version version = Version::HTTP1_1, std::function<void()>&& close = [](){}, std::function<std::string_view()>&& move = [](){return "localhost";}, std::function<bool()>&& reset = [](){return false;})
                : stream{stream}
                , version{version}
                , close{std::move(close)}
                , host{std::move(move)}
                , reset{std::move(reset)}
                , closed(false)
            {}

            template<typename D>
            D get(ClientRequest const& request)                  const   {return processes<Method::GET, D>(request, 0);}
            template<typename D, typename S>
            D put(ClientRequest const& request, S const& src)    const   {return processes<Method::PUT, D>(request, src);}
            template<typename D, typename S>
            D post(ClientRequest const& request, S const& src)   const   {return processes<Method::POST, D>(request, src);}

            using AsyncAction = std::function<void(ClientHTTPResponse const&)>;
            void get_async(ClientRequest const& request, AsyncAction&& action)                  const   {return processes_async<Method::GET>(request, 0, std::forward<AsyncAction>(action));}
            template<typename S>
            void put_async(ClientRequest const& request, S const& src, AsyncAction&& action)    const   {return processes_async<Method::PUT>(request, src, std::forward<AsyncAction>(action));}
            template<typename S>
            void post_async(ClientRequest const& request, S const& src, AsyncAction&& action)   const   {return processes_async<Method::POST>(request, src, std::forward<AsyncAction>(action));}

            void send(Method method, ClientRequest const& request, BodyEncoding encoding, std::function<void(StreamOutput& action)>&& action) const;
            void processResp(std::function<void(ClientHTTPResponse const&)>&& action) const;
        private:
            template<Method method, typename S>
            void processes_async(ClientRequest const& request, S const& src, AsyncAction&& action) const
            {
                for (int retry = 0; retry < 5; ++retry)
                {
                    if constexpr (method == Method::GET)
                    {
                        send(Method::GET, request, 0, [](StreamOutput&){});
                    }
                    else
                    {
                        send(method, request, ThorsAnvil::Serialize::jsonStreamSize(src), [&src](std::ostream& output)
                        {
                            output << ThorsAnvil::Serialize::jsonExporter(src);
                        });
                    }
                    processResp(std::forward<AsyncAction>(action));
                    if (stream.eof() && reset()) {
                        closed = false;
                        continue;
                    }
                    return;
                }
                // Failed 5 times log and throw.
                ThorsLogAndThrowError(std::runtime_error, "ThorsAnvil::Nisse::HTTP::ClientHTTPBase", "processes", "Failed to extract value from input stream");
            }
            template<Method method, typename D, typename S>
            D processes(ClientRequest const& request, S const& src) const
            {
                D result;
                processes_async<method, S>(request, src, [&result](ClientHTTPResponse const& resp)
                {
                    resp.body() >> ThorsAnvil::Serialize::jsonImporter(result);
                });
                return result;
            }
            void sendHTTP(Method method, ClientRequest const& request, BodyEncoding encoding) const;

    };
    class ClientHTTP: public ClientHTTPBase
    {
        struct GetHostName
        {
            std::string_view operator()(ThorsAnvil::ThorsSocket::FileInfo const& /*init*/)      {return "Ignore";}
            std::string_view operator()(ThorsAnvil::ThorsSocket::PipeInfo const& /*init*/)      {return "Ignore";}
            std::string_view operator()(ThorsAnvil::ThorsSocket::SocketInfo const& init)        {return init.host;}
            std::string_view operator()(ThorsAnvil::ThorsSocket::SocketService const& init)     {return init.host;}
            std::string_view operator()(ThorsAnvil::ThorsSocket::SSocketInfo const& init)       {return init.host;}
            std::string_view operator()(ThorsAnvil::ThorsSocket::SSocketService const& init)    {return init.host;}
        };
        ThorsAnvil::ThorsSocket::SocketInit     init;
        ThorsAnvil::ThorsSocket::SocketStream   stream;

        public:
            ClientHTTP(ThorsAnvil::ThorsSocket::SSocketInfo const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, version, [&](){stream.close();}, [&](){return hostname();}, [&](){return resetStream();}}
                , init{info}
                , stream{init}
            {}
            ClientHTTP(ThorsAnvil::ThorsSocket::SocketInfo const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, version, [&](){stream.close();}, [&](){return hostname();}, [&](){return resetStream();}}
                , init{info}
                , stream{init}
            {}
            ClientHTTP(ThorsAnvil::ThorsSocket::SocketService const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, version, [&](){stream.close();}, [&](){return hostname();}, [&](){return resetStream();}}
                , init{info}
                , stream{init}
            {}
            ClientHTTP(ThorsAnvil::ThorsSocket::SSocketService const& info, Version version = Version::HTTP1_1)
                : ClientHTTPBase{stream, version, [&](){stream.close();}, [&](){return hostname();}, [&](){return resetStream();}}
                , init{info}
                , stream{init}
            {}
            std::string_view hostname() const {return std::visit(GetHostName{}, init);}
            bool resetStream()
            {
                stream = ThorsAnvil::ThorsSocket::SocketStream{init};
                return true;
            }
    };
}

#endif
