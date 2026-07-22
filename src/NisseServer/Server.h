#ifndef THORSANVIL_NISSE_SERVER_H
#define THORSANVIL_NISSE_SERVER_H

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

#include "NisseServerConfig.h"
#include "JobQueue.h"
#include "Store.h"
#include "EventHandler.h"
#include <ThorsSocket/SocketStream.h>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace TASock = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class Context;

class Server
{
    friend class Context;

    JobQueue                        jobQueue;
    Store                           store;
    EventHandler                    eventHandler;

    public:
        Server(std::size_t workerCount = 1);

        void run(std::function<void()>&& notice = [](){});
        void stopSoft();
        void stopHard();
        bool isFeatureEnabled(Feature feature) const {return eventHandler.isFeatureEnabled(feature);}
        void listen(TASock::ServerInit&& listenerInit, Pynt& pynt);
        template<typename T, typename rep>
        int  addTimer(std::chrono::duration<T, rep> time, TimerAction& action)
        {
            return eventHandler.addTimer(std::chrono::microseconds(time).count(), action);
        }

    private:
        CoRoutine  createStreamJob(StreamData& info);
        CoRoutine  createAcceptJob(ServerData& info);
};

namespace UnitTest
{
    // Run a server inside a thread.
    // When we hit the destructor
    //      1: ask the server to stop
    //      2: wait for the thread to exit.
    template<typename T>
    class ServerRunner
    {
        T*                      activeServer;
        std::mutex              mutex;
        std::condition_variable condvar;
        std::thread             runner;
        public:
            template<typename... Args>
            ServerRunner(Args&&... args)
                : activeServer{nullptr}
                , runner([&]()
                {
                    T   server{std::forward<Args>(args)...};
                    server.run([&]()
                               {
                                    std::unique_lock<std::mutex>  lock{mutex};
                                    activeServer = &server;
                                    condvar.notify_one();
                               });
                    std::unique_lock<std::mutex>    lock{mutex};
                    activeServer = nullptr;
                })
            {
                std::unique_lock<std::mutex>     lock{mutex};
                condvar.wait(lock, [&](){return activeServer != nullptr;});
            }
            ~ServerRunner()
            {
                {
                    std::unique_lock<std::mutex>     lock{mutex};
                    if (activeServer) {
                        activeServer->stopHard();
                    }
                }
                runner.join();
            }
            void stopHard()
            {
                std::unique_lock<std::mutex>     lock{mutex};
                if (activeServer) {
                    activeServer->stopHard();
                }
            }
            void stopSoft()
            {
                std::unique_lock<std::mutex>     lock{mutex};
                if (activeServer) {
                    activeServer->stopSoft();
                }
            }
            void listen(TASock::ServerInit&& listenerInit, Pynt& pynt)
            {
                std::unique_lock<std::mutex>     lock{mutex};
                if (activeServer) {
                    activeServer->listen(std::forward<TASock::ServerInit>(listenerInit), pynt);
                }
            }
    };
};

}

#if defined(NISSE_HEADER_ONLY) && NISSE_HEADER_ONLY == 1
#include "Context.h"
#include "Context.source"
#include "PyntControl.h"
#include "PyntControl.source"
#include "Server.source"
#endif

#endif
