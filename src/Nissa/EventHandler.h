#ifndef THORSANVIL_NISSA_EVENT_HANDLER_H
#define THORSANVIL_NISSA_EVENT_HANDLER_H

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

#include "NissaConfig.h"
#include "Action.h"
#include "EventHandlerLibEvent.h"
#include "Store.h"

/*
 * C-Callback registered with LibEvent
 */
extern "C" void eventCallback(evutil_socket_t fd, short eventType, void* data);
extern "C" void controlTimerCallback(evutil_socket_t fd, short eventType, void* data);

namespace ThorsAnvil::Nissa
{

class JobQueue;
class Store;
struct StreamData;
struct ServerData;

class EventHandler
{
    static constexpr int controlTimerPause = 1000;  // microseconds between iterations.

    JobQueue&       jobQueue;
    Store&          store;
    EventBase       eventBase;
    Event           timer;

    public:
        EventHandler(JobQueue& jobQueue, Store& store);

        void run();
        void add(ThorsAnvil::ThorsSocket::Server&& stream, ServerTask&& task, ServerCreator&& creator);
        void add(ThorsAnvil::ThorsSocket::SocketStream&& stream, StreamTask&& task, StreamCreator&& creator);

    private:
        friend void ::eventCallback(evutil_socket_t fd, short eventType, void* data);
        void eventHandle(int fd, EventType type);

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
            void operator()(ServerData& info) {handler.addJob(info.coRoutine, fd);}
            void operator()(StreamData& info) {if (handler.checkFileDescriptorOK(fd, type)) {handler.addJob(info.coRoutine, fd);}}

        };
        bool checkFileDescriptorOK(int fd, EventType type);
        void addJob(CoRoutine& work, int fd);
};

}

#endif
