#ifndef THORSANVIL_NISSE_SERVER_H
#define THORSANVIL_NISSE_SERVER_H

/*
 * NisseServer:
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

#include "NisseConfig.h"
#include "JobQueue.h"
#include "Store.h"
#include "EventHandler.h"
#include "Pynt.h"
#include <ThorsSocket/SocketStream.h>

namespace ThorsAnvil::Nisse
{

class NisseServer
{
    JobQueue                        jobQueue;
    Store                           store;
    EventHandler                    eventHandler;

    public:
        NisseServer(int workerCount = 1);

        void run();
        template<typename T>
        void listen(T listenerInit, Pynt& pynt);

    private:
        CoRoutine  createStreamJob(StreamData& info);
        CoRoutine  createAcceptJob(ServerData& info);
};

}

#endif
