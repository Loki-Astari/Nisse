#include "EventHandler.h"
#include "JobQueue.h"
#include "Store.h"
#include "Action.h"

/*
 * C Callback functions.
 * Simply decide the data into EventHandler and call the C++ functions.
 */
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

/*
 * EventLib wrapper. Set up C-Function callbacks
 */
Event::Event(EventBase& eventBase, int fd, short type, EventHandler& eventHandler)
    : event{event_new(eventBase.eventBase, fd, type, &eventCallback, &eventHandler)}
{}

Event::Event(EventBase& eventBase, EventHandler& eventHandler)
    : event{evtimer_new(eventBase.eventBase, controlTimerCallback, &eventHandler)}
{}

EventHandler::EventHandler(JobQueue& jobQueue, Store& store)
    : jobQueue(jobQueue)
    , store(store)
    , timer(eventBase, *this)
{
    timer.add(controlTimerPause);
}

void EventHandler::run()
{
    eventBase.run();
}

void EventHandler::add(ThorsAnvil::ThorsSocket::Server&& server, ServerTask&& task, ServerCreator&& serverCreator)
{
    int fd = server.socketId();
    store.requestChange(StateUpdateCreateServer{fd,
                                                std::move(server),
                                                std::move(task),
                                                std::move(serverCreator),
                                                Event{eventBase, fd, EV_READ, *this},
                                               });
}

void EventHandler::add(ThorsAnvil::ThorsSocket::SocketStream&& stream, StreamTask&& task, StreamCreator&& streamCreator)
{
    int fd = stream.getSocket().socketId();
    store.requestChange(StateUpdateCreateStream{fd,
                                                std::move(stream),
                                                std::move(task),
                                                std::move(streamCreator),
                                                Event{eventBase, fd, EV_READ, *this},
                                                Event{eventBase, fd, EV_WRITE, *this},
                                               });
}

void EventHandler::eventHandle(int fd, EventType type)
{
    StoreData& info = store.getStoreData(fd);
    std::visit(ApplyEvent{*this, fd, type},  info);
}

bool EventHandler::checkFileDescriptorOK(int fd, EventType type)
{
    /*
     * This function detects if the socket has been closed at the other end.
     * This will induce a read event but there will be no data on the stream.
     */
    if (type == EventType::Write) {
        return true;
    }
    char buffer;
    ssize_t result = recv(fd, &buffer, 1, MSG_PEEK);
    if (result == 0 || (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
    {
        std::cout << "Remove Socket\n";
        store.requestChange(StateUpdateRemove{fd});
        return false;
    }
    return true;
}

void EventHandler::addJob(CoRoutine& work, int fd)
{
    jobQueue.addJob([&work, fd, &store = this->store]()
    {
        TaskYieldState task = TaskYieldState::Remove;
        if (work()) {
            task = work.get();
        }
        switch (task)
        {
            case TaskYieldState::Remove:
                store.requestChange(StateUpdateRemove{fd});
                break;
            case TaskYieldState::RestoreRead:
                store.requestChange(StateUpdateRestoreRead{fd});
                break;
            case TaskYieldState::RestoreWrite:
                store.requestChange(StateUpdateRestoreWrite{fd});
                break;
        }
    });
}

/*
 * Callback function controlled by timer.
 * This is used to processes all pending changes.
 *
 * Thus all changes to state are done by the main thread.
 */
void EventHandler::controlTimerAction()
{
    // Update all the state information.
    store.processUpdateRequest();
    // Put the timer back.
    timer.add(controlTimerPause);
}
