#include "EventHandler.h"

void eventCallback(evutil_socket_t fd, short eventType, void* data)
{
    ThorsAnvil::Nissa::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nissa::EventHandler*>(data);
    eventHandler.eventHandle(fd, static_cast<ThorsAnvil::Nissa::EventType>(eventType));
}

using namespace ThorsAnvil::Nissa;

EventHandler::EventHandler()
    : eventBase(event_base_new())
{}

EventHandler::~EventHandler()
{
    for (auto& event: tracking)
    {
        event_del(event.second.event);
    }
    event_base_free(eventBase);
}

void EventHandler::run()
{
    event_base_loop(eventBase, EVLOOP_NO_EXIT_ON_EMPTY);
}

void EventHandler::add(int fd, EventType eventType, EventAction&& action)
{
    EventDef    eventDef{fd, eventType};
    auto find = tracking.find(eventDef);

    if (find == tracking.end())
    {
        Event* event = event_new(eventBase, eventDef.fd, static_cast<short>(eventDef.type), &eventCallback, this);
        auto const [iter, ok] = tracking.insert({eventDef, EventInfo{event, std::move(action)}});
        find = iter;
    }
    else
    {
        find->second.action = std::move(action);
    }

    EventInfo& info = find->second;
    event_add(info.event, nullptr);
}

void EventHandler::eventHandle(int fd, EventType type)
{
    auto find = tracking.find(EventDef{fd, type});
    if (find != tracking.end())
    {
        EventInfo& info = find->second;
        info.action();
        event_add(info.event, nullptr);
    }
}
