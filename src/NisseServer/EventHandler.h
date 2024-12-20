#ifndef THORSANVIL_NISSE_EVENT_HANDLER_H
#define THORSANVIL_NISSE_EVENT_HANDLER_H

/*
 * A thin wrapper on libEvent to C++ it.
 *
 * When an socket listener is first created via add() we store all data in the Store object.
 * When this has been created it adds the `ReadEvent` to libEvent to listen for any data.
 *
 * When (if) a socket event is triggered we save a lambda on the JobQueue addJob() that will be
 * executed by a thread. The lambda restarts the CoRoutine which will either yield one of
 * three values.
 *
 * When the code yields one of three situations happens:
 *      * TaskYieldState::RestoreRead    We restore the read listener waiting for more data.
 *      * TaskYieldState::RestoreWrite   We restore the write listener waiting for space to write.
 *      * TaskYieldState::Remove         We destroy the socket and all associated data.
 *
 * Note: This data is never destroyed immediately because the code may be executing on any thread.
 *       Instead a request is queued on the `Store` object. The main thread will then be used
 *       to clean up data (See Store for details).
 */

#include "NisseConfig.h"
#include "NisseUtil.h"
#include "EventHandlerLibEvent.h"
#include "Store.h"

/*
 * C-Callback registered with LibEvent
 */
extern "C" void eventCallback(evutil_socket_t fd, short eventType, void* data);
extern "C" void controlTimerCallback(evutil_socket_t fd, short eventType, void* data);

namespace TASock   = ThorsAnvil::ThorsSocket;

namespace ThorsAnvil::Nisse::Server
{

class JobQueue;
class Store;
struct StreamData;
struct ServerData;
struct OwnedFD;

class EventHandler
{
    static constexpr int controlTimerPause = 1000;  // microseconds between iterations.

    JobQueue&       jobQueue;
    Store&          store;
    EventBase       eventBase;
    Event           timer;
    bool            finished;

    public:
        EventHandler(JobQueue& jobQueue, Store& store);

        void run();
        void stop();
        void add(TASock::Server&& stream, ServerCreator&& creator, Pynt& pynt);
        void add(TASock::SocketStream&& stream, StreamCreator&& creator, Pynt& pynt);
        void addOwnedFD(int fd, int owner, EventType initialWait);
        void remOwnedFD(int fd);
        void addSharedFD(int fd);
        void remSharedFD(int fd);

    private:
        friend void ::eventCallback(evutil_socket_t fd, short eventType, void* data);
        void eventAction(int fd, EventType type);

    private:
        friend void ::controlTimerCallback(evutil_socket_t fd, short eventType, void* data);
        void controlTimerAction();

    private:
        struct ApplyEvent
        {
            EventHandler&   handler;
            int             fd;
            EventType       type;
            ApplyEvent(EventHandler& handler, int fd, EventType type)
                : handler(handler)
                , fd(fd)
                , type(type)
            {}
            void operator()(ServerData& info)       {handler.handleServerEvent(info, fd, type);}
            void operator()(StreamData& info)       {handler.handleStreamEvent(info, fd, type);}
            void operator()(OwnedFD& info)          {handler.handleLinkStreamEvent(info, fd, type);}
            void operator()(SharedFD& info)         {handler.handlePipeStreamEvent(info, fd, type);}
        };

        // --- Handlers
        void handleServerEvent(ServerData& info, int fd, EventType type);
        void handleStreamEvent(StreamData& info, int fd, EventType type);
        void handleLinkStreamEvent(OwnedFD& info, int fd, EventType type);
        void handlePipeStreamEvent(SharedFD& info, int fd, EventType type);
        // --- Handler Utility
        bool checkFileDescriptorOK(int fd, EventType type);
        void addJob(CoRoutine& work, int fd);
};

}

#endif
