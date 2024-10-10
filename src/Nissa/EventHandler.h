#ifndef THORSANVIL_NISSA_EVENT_HANDLER_H
#define THORSANVIL_NISSA_EVENT_HANDLER_H

#include "NissaConfig.h"
#include "EventHandlerLibEvent.h"
#include <event2/event.h>
#include <map>
#include <tuple>
#include <functional>
#include <mutex>

extern "C" void eventCallback(evutil_socket_t fd, short eventType, void* data);
extern "C" void controlTimerCallback(evutil_socket_t fd, short eventType, void* data);

namespace ThorsAnvil::Nissa
{

enum class EventType : short{Read = EV_READ, Write = EV_WRITE};
enum class EventTask        {Create, Restore, Remove};

using EventAction   = std::function<void(bool)>;


struct EventDef
{
    int         fd;
    EventType   type;
    friend bool operator<(EventDef const& lhs, EventDef const& rhs) {return std::tie(lhs.fd, lhs.type) < std::tie(rhs.fd, rhs.type);}
};
struct EventInfo
{
    Event       event;
    EventAction action;
};
struct EventUpdate
{
    EventDef    eventDef;
    EventTask   task;
    EventAction action;
};

using EventMap          = std::map<EventDef, EventInfo>;
using EventUpdateList   = std::vector<EventUpdate>;

class EventHandler
{
    static constexpr int ControlTimerPause = 1000;  // microsends between iterations.

    EventBase       eventBase;
    EventMap        tracking;
    std::mutex      updateListMutex;
    EventUpdateList updateList;
    Event           timer;

    public:
        EventHandler();

        void run();
        void add(int fd, EventType eventType, EventAction&& action);
        void restore(int fd, EventType eventType);
        void remove(int fd, EventType eventType);
    private:
        friend void ::eventCallback(evutil_socket_t fd, short eventType, void* data);
        void eventHandle(int fd, EventType type);

    private:
        friend void ::controlTimerCallback(evutil_socket_t fd, short eventType, void* data);
        void controlTimerAction();

        bool checkFileDescriptorOK(int fd, EventType type);
};

}

#endif
