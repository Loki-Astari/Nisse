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
Event::Event(EventBase& eventBase, int fd, EventType type, EventHandler& eventHandler)
    : event{event_new(eventBase.eventBase, fd, static_cast<short>(type), &eventCallback, &eventHandler)}
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

void EventHandler::add(ThorsAnvil::ThorsSocket::Server&& server, ServerCreator&& serverCreator, Pint& pint)
{
    int fd = server.socketId();
    store.requestChange(StateUpdateCreateServer{fd,
                                                std::move(server),
                                                std::move(serverCreator),
                                                Event{eventBase, fd, EventType::Read, *this},
                                                pint
                                               });
}

void EventHandler::add(ThorsAnvil::ThorsSocket::SocketStream&& stream, StreamCreator&& streamCreator, Pint& pint)
{
    int fd = stream.getSocket().socketId();
    store.requestChange(StateUpdateCreateStream{fd,
                                                std::move(stream),
                                                std::move(streamCreator),
                                                Event{eventBase, fd, EventType::Read, *this},
                                                Event{eventBase, fd, EventType::Write, *this},
                                                pint
                                               });
}

void EventHandler::eventHandle(int fd, EventType type)
{
    StoreData& info = store.getStoreData(fd);
    std::visit(ApplyEvent{*this, fd, type},  info);
    /* The std::visit ApplyEvent object to call checkFileDescriptorOK()/addJob() below */
}

bool EventHandler::checkFileDescriptorOK(int fd, EventType type)
{
    /*
     * This function detects if the socket has been closed at the other end.
     * If the other end of the socket was closed this will induce a read event.
     * But no data will be available.
     */
    if (type == EventType::Write) {
        return true;
    }
    char buffer;
    ssize_t result = recv(fd, &buffer, 1, MSG_PEEK);
    if (result == 0 || (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
    {
        // Remove a socket if the other end has been closed.
        std::cout << "Remove Socket\n";
        store.requestChange(StateUpdateRemove{fd});
        return false;
    }
    return true;
}

void EventHandler::addJob(CoRoutine& work, int fd)
{
    /*
     * Add a job to the task queue.
     * This calls the passed CoRoutine and then updates the state of the object.
     * Note: The update is done via requestChange() as it can be called by any thread.
     */
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
