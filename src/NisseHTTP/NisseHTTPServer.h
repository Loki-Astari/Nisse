#ifndef THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H
#define THORSANVIL_NISSE_HTTP_NISSEHTTP_SERVER_H

<<<<<<< HEAD
=======
#include <thread>
>>>>>>> 89d419c (Add NisseHTTPServer)
#include "PyntHTTPControl.h"
#include "HTTPHandler.h"
#include "NisseServer/NisseServer.h"

namespace ThorsAnvil::Nisse::HTTP
{

class NisseHTTPServer
{
    Server::NisseServer     server;
    HTTPHandler             handler;
    PyntHTTPControl         control;
    std::thread             thread;

    public:
        NisseHTTPServer(std::size_t workerCount, TASock::ServerInit&& handlerInit, TASock::ServerInit&& controlInit)
            : server(workerCount)
            , control(server)
            , thread{[&](){run();}}
        {
            server.listen(std::move(handlerInit), handler);
            server.listen(std::move(controlInit), control);
        }
        ~NisseHTTPServer()
        {
            server.stopHard();
            thread.join();
        }

        void run()                          {server.run();}
        void stopSoft()                     {server.stopSoft();}
        void stopHard()                     {server.stopHard();}
        Server::NisseServer& getServer()    {return server;}

        void addPath(MethodChoice method, std::string const& path, HTTPAction&& action, HTTPValidate&& val = [](Request const&){return true;})
        {
            handler.addPath(method, path, std::forward<HTTPAction>(action), std::forward<HTTPValidate>(val));
        }
};

}

#endif
