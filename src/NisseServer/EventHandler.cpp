#include "EventHandler.h"
#include "NisseUtil.h"
#include "JobQueue.h"
#include "Store.h"
#include <ThorsLogging/ThorsLogging.h>

/*
 * C Callback functions.
 * Simply decide the data into EventHandler and call the C++ functions.
 */
void eventCallback(evutil_socket_t fd, short eventType, void* data)
{
    ThorsAnvil::Nisse::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nisse::EventHandler*>(data);
    eventHandler.eventAction(fd, static_cast<ThorsAnvil::Nisse::EventType>(eventType));
}

void controlTimerCallback(evutil_socket_t, short, void* data)
{
    ThorsAnvil::Nisse::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nisse::EventHandler*>(data);
    eventHandler.controlTimerAction();
}

namespace TAS   = ThorsAnvil::ThorsSocket;
using namespace ThorsAnvil::Nisse;

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
    : jobQueue{jobQueue}
    , store{store}
    , timer{eventBase, *this}
    , finished{false}
{
    timer.add(controlTimerPause);
}

void EventHandler::run()
{
    finished = false;
    eventBase.run();
}

void EventHandler::stop()
{
    finished = true;
}

void EventHandler::add(TAS::Server&& server, ServerCreator&& serverCreator, Pynt& pynt)
{
    int fd = server.socketId();
    store.requestChange(StateUpdateCreateServer{fd,
                                                std::move(server),
                                                std::move(serverCreator),
                                                Event{eventBase, fd, EventType::Read, *this},
                                                pynt
                                               });
}

void EventHandler::add(TAS::SocketStream&& stream, StreamCreator&& streamCreator, Pynt& pynt)
{
    int fd = stream.getSocket().socketId();
    store.requestChange(StateUpdateCreateStream{fd,
                                                std::move(stream),
                                                std::move(streamCreator),
                                                Event{eventBase, fd, EventType::Read, *this},
                                                Event{eventBase, fd, EventType::Write, *this},
                                                pynt
                                               });
}

void EventHandler::addLinkedStream(int fd, int owner, EventType initialWait)
{
    store.requestChange(StateUpdateCreateLinkStream{fd,
                                                    owner,
                                                    initialWait,
                                                    Event{eventBase, fd, EventType::Read, *this},
                                                    Event{eventBase, fd, EventType::Write, *this},
                                                   });
}

void EventHandler::remLinkedStream(int fd)
{
    store.requestChange(StateUpdateRemove{fd});
}

void EventHandler::eventAction(int fd, EventType type)
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
        ThorsLogInfo("ThorsAnvil::Nissa::EventHandler::", "checkFileDescriptorOK", "Client closed connection");
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
        TaskYieldAction task = {TaskYieldState::Remove, fd};
        try
        {
            if (work()) {
                task = work.get();
            }
        }
        catch (std::exception const& e)
        {
            ThorsLogWarning("ThorsAnvil::Nissa::EventHandler::", "addJob", "jobQueue::job: Ignoring Exception: ",  e.what());
        }
        catch (...)
        {
            ThorsLogWarning("ThorsAnvil::Nissa::EventHandler::", "addJob", "jobQueue::job: Ignoring Exception: Unknown");
        }
        switch (task.state)
        {
            case TaskYieldState::Remove:
                store.requestChange(StateUpdateRemove{task.fd});
                break;
            case TaskYieldState::RestoreRead:
                store.requestChange(StateUpdateRestoreRead{task.fd});
                break;
            case TaskYieldState::RestoreWrite:
                store.requestChange(StateUpdateRestoreWrite{task.fd});
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
    if (finished)
    {
        eventBase.loopBreak();
        return;
    }

    // Update all the state information.
    store.processUpdateRequest();
    // Put the timer back.
    timer.add(controlTimerPause);
}
