#ifndef THORSANVIL_NISSA_EVENT_HANDLER_H
#define THORSANVIL_NISSA_EVENT_HANDLER_H

#include "NissaConfig.h"
#include "EventHandlerLibEvent.h"
#include "ThorsSocket/SocketStream.h"
#include <event2/event.h>
#include <boost/coroutine2/all.hpp>
#include <map>
#include <tuple>
#include <functional>
#include <mutex>

extern "C" void eventCallback(evutil_socket_t fd, short eventType, void* data);
extern "C" void controlTimerCallback(evutil_socket_t fd, short eventType, void* data);

namespace ThorsAnvil::Nissa
{

class JobQueue;

enum class EventType : short{Read = EV_READ, Write = EV_WRITE, Ignore = 0};
enum class EventTask        {Create, RestoreRead, RestoreWrite, Remove};

using CoRoutine     = boost::coroutines2::coroutine<EventTask>::pull_type;
using Yield         = boost::coroutines2::coroutine<EventTask>::push_type;
using EventAction   = std::function<void(ThorsAnvil::ThorsSocket::SocketStream& stream, Yield& yield)>;


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

struct EventUpdate
{
    using SocketStream  = ThorsAnvil::ThorsSocket::SocketStream;

    int             fd;
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

        bool checkFileDescriptorOK(int fd, EventType type);
        CoRoutine buildCoRoutine(EventInfo& action);
};

}

#endif
