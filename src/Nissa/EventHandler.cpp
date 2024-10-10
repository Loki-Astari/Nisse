#include "EventHandler.h"
#include "JobQueue.h"

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

EventHandler::EventHandler(JobQueue& jobQueue)
    : jobQueue(jobQueue)
    , timer(eventBase, *this)
{
    timer.add(ControlTimerPause);
}

void EventHandler::run()
{
    eventBase.run();
}

void EventHandler::add(int fd, ThorsAnvil::ThorsSocket::SocketStream&& stream, EventAction&& action)
{
    std::unique_lock    lock(updateListMutex);
    updateList.emplace_back(fd, EventTask::Create, std::move(action), std::move(stream));
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
    auto find = tracking.find(fd);
    if (find != tracking.end())
    {
        EventInfo& info = find->second;

        if (info.stream.getSocket().isConnected() && !checkFileDescriptorOK(fd, type))
        {
            std::cerr << "Remove Socket\n";
            std::unique_lock    lock(updateListMutex);
            updateList.emplace_back(fd, EventTask::Remove, EventAction{}, ThorsAnvil::ThorsSocket::SocketStream{});
            return;
        }
        jobQueue.addJob([&]()
        {
            EventTask task = info.action(info.stream);
            std::unique_lock    lock(updateListMutex);
            updateList.emplace_back(fd, task, EventAction{}, ThorsAnvil::ThorsSocket::SocketStream{});
        });
    }
}

void EventHandler::controlTimerAction()
{
    std::unique_lock    lock(updateListMutex);
    for (auto& eventUpdate: updateList)
    {
        switch (eventUpdate.task)
        {
            case EventTask::Create:
            {
                auto find = tracking.find(eventUpdate.fd);

                if (find == tracking.end())
                {
                    Event readEvent(eventBase, eventUpdate.fd, EV_READ, *this);
                    Event writeEvent(eventBase, eventUpdate.fd, EV_WRITE, *this);
                    auto const [iter, ok] = tracking.insert({eventUpdate.fd, EventInfo{std::move(readEvent), std::move(writeEvent), std::move(eventUpdate.action), std::move(eventUpdate.stream)}});
                    find = iter;
                }
                else
                {
                    find->second.action = std::move(eventUpdate.action);
                    find->second.stream = std::move(eventUpdate.stream);
                }

                find->second.readEvent.add();
                break;
            }
            case EventTask::RestoreRead:
            {
                auto find = tracking.find(eventUpdate.fd);
                if (find != tracking.end()) {
                    find->second.readEvent.add();
                }
                break;
            }
            case EventTask::RestoreWrite:
            {
                auto find = tracking.find(eventUpdate.fd);
                if (find != tracking.end()) {
                    find->second.writeEvent.add();
                }
                break;
            }
            case EventTask::Remove:
            {
                auto find = tracking.find(eventUpdate.fd);
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
