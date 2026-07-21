#ifndef THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H
#define THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H

#include <thread>
#include "PyntHTTPControl.h"
#include "HTTPHandler.h"
#include "NisseServer/Server.h"

namespace ThorsAnvil::Nisse::HTTP
{

class NisseHTTPServer
{
    Server::Server          server;
    HTTPHandler             handler;
    PyntHTTPControl         control;

    public:
        NisseHTTPServer(std::size_t workerCount, TASock::ServerInit&& handlerInit, TASock::ServerInit&& controlInit)
            : server(workerCount)
            , control(server)
        {
            server.listen(std::move(handlerInit), handler);
            server.listen(std::move(controlInit), control);
        }
        ~NisseHTTPServer()
        {
            server.stopHard();
        }

        void run()                          {server.run();}
        void stopSoft()                     {server.stopSoft();}
        void stopHard()                     {server.stopHard();}
        Server::Server& getServer()         {return server;}

        void addPath(MethodChoice method, std::string const& path, HTTPAction&& action, HTTPValidate&& val = [](Request const&){return true;})
        {
            handler.addPath(method, path, std::forward<HTTPAction>(action), std::forward<HTTPValidate>(val));
        }
};

}

#endif
