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

Event::Event(EventBase& eventBase, int fd, short type, EventHandler& eventHandler)
    : event{event_new(eventBase.eventBase, fd, type, &eventCallback, &eventHandler)}
{}

Event::Event(EventBase& eventBase, EventHandler& eventHandler)
    : event{evtimer_new(eventBase.eventBase, controlTimerCallback, &eventHandler)}
{}

EventHandler::EventHandler()
    : timer(eventBase, *this)
{
    timer.add(ControlTimerPause);
}

void EventHandler::run()
{
    eventBase.run();
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
                    Event event(eventBase, eventDef.fd, static_cast<short>(eventDef.type), *this);
                    auto const [iter, ok] = tracking.insert({eventDef, EventInfo{std::move(event), std::move(eventUpdate.action)}});
                    find = iter;
                }
                else
                {
                    find->second.action = std::move(eventUpdate.action);
                }

                find->second.event.add();
                break;
            }
            case EventTask::Restore:
            {
                auto find = tracking.find(eventDef);
                if (find != tracking.end()) {
                    find->second.event.add();
                }
                break;
            }
            case EventTask::Remove:
            {
                auto find = tracking.find(eventDef);
                if (find != tracking.end()) {
                    tracking.erase(find);
                }
                break;
            }
        }
    }
    updateList.clear();
    timer.add(ControlTimerPause);
}
