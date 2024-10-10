#include "EventHandler.h"

void eventCallback(evutil_socket_t fd, short eventType, void* data)
{
    ThorsAnvil::Nissa::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nissa::EventHandler*>(data);
    eventHandler.eventHandle(fd, static_cast<ThorsAnvil::Nissa::EventType>(eventType));
}

void controlTimerCallback(evutil_socket_t, short, void* data)
{
    ThorsAnvil::Nissa::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nissa::EventHandler*>(data);
    eventHandler.controlTimerAction();
}

using namespace ThorsAnvil::Nissa;

EventHandler::EventHandler()
    : eventBase(event_base_new())
{
    timer = evtimer_new(eventBase, controlTimerCallback, this);
    TimeOut timeout = {0, ControlTimerPause};
    evtimer_add(timer, &timeout);
}

EventHandler::~EventHandler()
{
    for (auto& event: tracking)
    {
        event_del(event.second.event);
    }
    evtimer_del(timer);
    event_base_free(eventBase);
}

void EventHandler::run()
{
    event_base_loop(eventBase, EVLOOP_NO_EXIT_ON_EMPTY);
}

void EventHandler::add(int fd, EventType eventType, EventAction&& action)
{
    std::unique_lock    lock(updateListMutex);
    updateList.emplace_back(EventDef{fd, eventType}, EventTask::Create, std::move(action));
}
void EventHandler::restore(int fd, EventType eventType)
{
    std::unique_lock    lock(updateListMutex);
    updateList.emplace_back(EventDef{fd, eventType}, EventTask::Restore, [](bool){});
}
void EventHandler::remove(int fd, EventType eventType)
{
    std::unique_lock    lock(updateListMutex);
    updateList.emplace_back(EventDef{fd, eventType}, EventTask::Remove, [](bool){});
}

bool EventHandler::checkFileDescriptorOK(int fd, EventType type)
{
    if (type == EventType::Write) {
        return true;
    }
    char buffer;
    ssize_t result = recv(fd, &buffer, 1, MSG_PEEK);
    return !(result == 0 || (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK));
}

void EventHandler::eventHandle(int fd, EventType type)
{
    auto find = tracking.find(EventDef{fd, type});
    if (find != tracking.end())
    {
        EventInfo& info = find->second;
        info.action(checkFileDescriptorOK(fd, type));
    }
}
void EventHandler::controlTimerAction()
{
    std::unique_lock    lock(updateListMutex);
    for (auto const& eventUpdate: updateList)
    {
        EventDef const&    eventDef = eventUpdate.eventDef;
        switch (eventUpdate.task)
        {
            case EventTask::Create:
            {
                auto find = tracking.find(eventDef);

                if (find == tracking.end())
                {
                    Event* event = event_new(eventBase, eventDef.fd, static_cast<short>(eventDef.type), &eventCallback, this);
                    auto const [iter, ok] = tracking.insert({eventDef, EventInfo{event, std::move(eventUpdate.action)}});
                    find = iter;
                }
                else
                {
                    find->second.action = std::move(eventUpdate.action);
                }

                EventInfo& info = find->second;
                event_add(info.event, nullptr);
                break;
            }
            case EventTask::Restore:
            {
                auto find = tracking.find(eventDef);
                if (find != tracking.end())
                {
                    EventInfo& info = find->second;
                    event_add(info.event, nullptr);
                }
                break;
            }
            case EventTask::Remove:
            {
                auto find = tracking.find(eventDef);
                if (find != tracking.end())
                {
                    EventInfo& info = find->second;
                    event_del(info.event);
                    tracking.erase(find);
                }
                break;
            }
        }
    }
    updateList.clear();
    TimeOut timeout = {0, ControlTimerPause};
    evtimer_add(timer, &timeout);
}
