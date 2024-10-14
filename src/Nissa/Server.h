#ifndef THORSANVIL_NISSA_SERVER_H
#define THORSANVIL_NISSA_SERVER_H

/*
 * Server:
 *  Holds
 *      JobQueue:       This is a set of background thread to do any work set by the user.
 *      Store:          All stage information needed by the server.
 *                      Storage is thread safe assuming:
 *                          Only main thread adds new data.
 *                          Each thread only reads the object that it is acting on.
 *      EventHandler:   LibEvent wrapper.
 *                      It hold's all the information needed to processes a connection.
 *  The server puts appropriate "lambdas" into the Event Handler to processes a socket.
 */

#include "NissaConfig.h"
#include "JobQueue.h"
#include "Store.h"
#include "EventHandler.h"
#include "Pint.h"
#include <ThorsSocket/SocketStream.h>

namespace ThorsAnvil::Nissa
{

class Server
{
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;

    JobQueue                        jobQueue;
    Store                           store;
    EventHandler                    eventHandler;

    public:
        Server(int workerCount = 1);

        void run();
        template<typename T>
        void listen(T listenerInit, Pint& pint);

    private:
        SocketStream getNextStream();
        void         connectionHandler();

        CoRoutine  createStreamJob(StreamData& info);
        CoRoutine  createAcceptJob(ServerData& info);
};

}

#endif
