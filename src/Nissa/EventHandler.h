#ifndef THORSANVIL_NISSA_EVENT_HANDLER_H
#define THORSANVIL_NISSA_EVENT_HANDLER_H

/*
 * A thin wrapper on libEvent to C++ it.
 *
 * For each socket track:
 *  1: SocketStream             std::iostream for reading/writting to socket.
 *  2: Task                     A lambda that will be execute when the SocketStream is not blocking.
 *  3: CoRoutine                A boost CoRoutine2 allows code to yield if the socket blocks.
 *                              ThorsSocket can be used with yielding so no extra code is needed.
 *
 * When an Event listener is first created we also add an event listener on libEvent and a
 * coroutine to track the state of Task and installs the yield object on the sockets.
 *
 * When (if) a socket event is triggered we save a lambda on the JobQueue that will be
 * executed by a thread. The lambda restarts the CoRoutine which will either yield one of
 * three values.
 *
 * When the code yields one of three situations happens:
 *      * TaskYieldState::RestoreRead    We restore the read listener waiting for more data.
 *      * TaskYieldState::RestoreWrite   We restore the write listener waiting for space to write.
 *      * TaskYieldState::Remove         We destroy the socket and all associated data.
 *
 * Note: This action is not done immediately the action is added to "updateList" which will
 *       be processed by the main thread every couple of milliseconds.
 *
 */

#include "NissaConfig.h"
#include "Action.h"
#include "EventHandlerLibEvent.h"
#include <event2/event.h>
#include <map>
#include <tuple>
#include <functional>
#include <mutex>

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

enum class EventType : short{Read = EV_READ, Write = EV_WRITE};

class EventHandler
{
    static constexpr int controlTimerPause = 1000;  // microsends between iterations.

    JobQueue&       jobQueue;
    Store&          store;
    EventBase       eventBase;
    Event           timer;

    public:
        EventHandler(JobQueue& jobQueue, Store& store);

        void run();
        void add(int fd, ThorsAnvil::ThorsSocket::SocketStream&& stream, Task&& task);

    private:
        friend void ::eventCallback(evutil_socket_t fd, short eventType, void* data);
        void eventHandle(int fd, EventType type);

    private:
        friend void ::controlTimerCallback(evutil_socket_t fd, short eventType, void* data);
        void controlTimerAction();

    private:
        bool checkFileDescriptorOK(int fd, EventType type);
        CoRoutine buildCoRoutine(StreamData& info);

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
            void operator()(ServerData& info) {handler(fd, type, info);}
            void operator()(StreamData& info) {handler(fd, type, info);}
        };
        void operator()(int fd, EventType type, ServerData& info);
        void operator()(int fd, EventType type, StreamData& info);
};

}

#endif
