#ifndef THORSANVIL_NISSA_EVENT_HANDLER_H
#define THORSANVIL_NISSA_EVENT_HANDLER_H

#include "NissaConfig.h"
#include <event2/event.h>
#include <map>
#include <tuple>
#include <functional>

extern "C" void eventCallback(evutil_socket_t fd, short eventType, void* data);

namespace ThorsAnvil::Nissa
{

using EventBase     = ::event_base;
using Event         = ::event;
using EventAction   = std::function<void()>;

enum class EventType : short {Read = EV_READ, Write = EV_WRITE};

struct EventDef
{
    int         fd;
    EventType   type;
    friend bool operator<(EventDef const& lhs, EventDef const& rhs) {return std::tie(lhs.fd, lhs.type) < std::tie(rhs.fd, rhs.type);}
};
struct EventInfo
{
    Event*      event;
    EventAction action;
};

using EventMap = std::map<EventDef, EventInfo>;

class EventHandler
{
    EventBase*      eventBase;
    EventMap        tracking;

    public:
        EventHandler();
        ~EventHandler();

        EventHandler(EventHandler const&)               = delete;
        EventHandler& operator=(EventHandler const&)    = delete;

        void run();
        void add(int fd, EventType eventType, EventAction&& action);
    private:
        friend void ::eventCallback(evutil_socket_t fd, short eventType, void* data);
        void eventHandle(int fd, EventType type);
};

}

#endif
