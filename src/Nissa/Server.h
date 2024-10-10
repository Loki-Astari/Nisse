#ifndef THORSANVIL_NISSA_SERVER_H
#define THORSANVIL_NISSA_SERVER_H

#include "NissaConfig.h"
#include "EventHandler.h"
#include "JobQueue.h"
#include "ThorsSocket/Server.h"
#include "ThorsSocket/SocketStream.h"
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ThorsAnvil::Nissa
{

class Server
{
    using SocketServer  = ThorsAnvil::ThorsSocket::Server;
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;
    using Certificate   = ThorsAnvil::ThorsSocket::CertificateInfo;
    using SSLctx        = ThorsAnvil::ThorsSocket::SSLctx;
    using Connections   = std::queue<SocketStream>;

    SSLctx                          ctx;
    std::vector<SocketServer>       listeners;
    EventHandler                    eventHandler;
    JobQueue                        jobQueue;

    public:
        Server(Certificate certificate, int workerCount = 1);

        void run();
        void listen(int port);

    private:
        SocketStream getNextStream();
        void         connectionHandler();

        Work createHttpJob(SocketStream&& acceptStream);
};

}

#endif
