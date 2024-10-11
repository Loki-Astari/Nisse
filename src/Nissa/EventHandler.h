#ifndef THORSANVIL_NISSA_EVENT_HANDLER_H
#define THORSANVIL_NISSA_EVENT_HANDLER_H

/*
 * A thin wrapper on libEvent to C++ it.
 *
 * For each socket track:
 *  1: SocketStream             std::iostream for reading/writting to socket.
 *  2: EventAction              A lambda that will be execute when the SocketStream is not blocking.
 *  3: CoRoutine                A boost CoRoutine2 allows code to yield if the socket blocks.
 *                              ThorsSocket can be used with yielding so no extra code is needed.
 *
 * When an Event listener is first created we also add an event listener on libEvent and a
 * coroutine to track the state of EventAction and installs the yield object on the sockets.
 *
 * When (if) a socket event is triggered we save a lambda on the JobQueue that will be
 * executed by a thread. The lambda restarts the CoRoutine which will either yield one of
 * three values.
 *
 * When the code yields one of three situations happens:
 *      * EventTask::RestoreRead    We restore the read listener waiting for more data.
 *      * EventTask::RestoreWrite   We restore the write listener waiting for space to write.
 *      * EventTask::Remove         We destroy the socket and all associated data.
 *
 * Note: This action is not done immediately the action is added to "updateList" which will
 *       be processed by the main thread every couple of milliseconds.
 *
 */

#include "NissaConfig.h"
#include "EventHandlerLibEvent.h"
#include <ThorsSocket/SocketStream.h>
#include <event2/event.h>
#include <boost/coroutine2/all.hpp>
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

enum class EventType : short{Read = EV_READ, Write = EV_WRITE};
enum class EventTask        {RestoreRead, RestoreWrite, Remove};

using CoRoutine     = boost::coroutines2::coroutine<EventTask>::pull_type;
using Yield         = boost::coroutines2::coroutine<EventTask>::push_type;
using EventAction   = std::function<void(ThorsAnvil::ThorsSocket::SocketStream& stream, Yield& yield)>;

/*
 * All the information we need to handle a socket.
 */
struct EventInfo
{
    static CoRoutine invalid;

    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;
    EventInfo(Event&& readEvent, Event&& writeEvent, EventAction&& action, SocketStream&& stream)
        : readEvent(std::move(readEvent))
        , writeEvent(std::move(writeEvent))
        , action(std::move(action))
        , stream(std::move(stream))
        , state{std::move(invalid)}
    {}
    EventInfo(EventInfo&& move)
        : readEvent(std::move(move.readEvent))
        , writeEvent(std::move(move.writeEvent))
        , action(std::move(move.action))
        , stream(std::move(move.stream))
        , state{std::move(move.state)}
    {}

    Event           readEvent;
    Event           writeEvent;
    EventAction     action;
    SocketStream    stream;
    CoRoutine       state;
};

/*
 * Object added to the "updateList" by any thread.
 * The main thread processes the list and updates the real state in "tracking"
 */
struct EventUpdate
{
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;

    int             fd;
    bool            create;
    EventTask       task;
    EventAction     action;
    SocketStream    stream;
};

using EventMap          = std::map<int, EventInfo>;
using EventUpdateList   = std::vector<EventUpdate>;

class EventHandler
{
    static constexpr int ControlTimerPause = 1000;  // microsends between iterations.

    JobQueue&       jobQueue;
    EventBase       eventBase;
    EventMap        tracking;
    std::mutex      updateListMutex;
    EventUpdateList updateList;
    Event           timer;

    public:
        EventHandler(JobQueue& jobQueue);

        void run();
        void add(int fd, ThorsAnvil::ThorsSocket::SocketStream&& stream, EventAction&& action);
    private:
        friend void ::eventCallback(evutil_socket_t fd, short eventType, void* data);
        void eventHandle(int fd, EventType type);

    private:
        friend void ::controlTimerCallback(evutil_socket_t fd, short eventType, void* data);
        void controlTimerAction();

    private:
        bool checkFileDescriptorOK(int fd, EventType type);
        CoRoutine buildCoRoutine(EventInfo& action);
};

}

#endif
