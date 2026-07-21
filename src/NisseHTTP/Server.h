#ifndef THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H
#define THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H

#include <thread>
#include "PyntHTTPControl.h"
#include "HTTPHandler.h"
#include "NisseServer/Server.h"

namespace ThorsAnvil::Nisse::HTTP
{

class Server: public ThorsAnvil::Nisse::Server::Server
{
    HTTPHandler             handler;
    PyntHTTPControl         control;

    public:
        Server(std::size_t workerCount, TASock::ServerInit&& handlerInit, TASock::ServerInit&& controlInit)
            : ThorsAnvil::Nisse::Server::Server(workerCount)
            , control(*this)
        {
            listen(std::move(handlerInit), handler);
            listen(std::move(controlInit), control);
        }
        ~Server()
        {
            stopHard();
        }

        void addPath(MethodChoice method, std::string const& path, HTTPAction&& action, HTTPValidate&& val = [](Request const&){return true;})
        {
            handler.addPath(method, path, std::forward<HTTPAction>(action), std::forward<HTTPValidate>(val));
        }
};

}

#endif
