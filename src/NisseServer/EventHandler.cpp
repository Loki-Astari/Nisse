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
    ThorsAnvil::Nisse::Server::EventHandler&    eventHandler = *reinterpret_cast<ThorsAnvil::Nisse::Server::EventHandler*>(data);
    eventHandler.eventAction(fd, static_cast<ThorsAnvil::Nisse::Server::EventType>(eventType));
}

void controlTimerCallback(evutil_socket_t, short eventType, void* data)
{
    ThorsAnvil::Nisse::Server::TimerData&    timerData = *reinterpret_cast<ThorsAnvil::Nisse::Server::TimerData*>(data);
    timerData.eventHandler->eventAction(timerData.timerId,static_cast<ThorsAnvil::Nisse::Server::EventType>(eventType));
    timerData.timerEvent.add(timerData.waitTime);
}

namespace TASock   = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

int EventHandler::nextTImerId = 1'000'000;

/*
 * EventLib wrapper. Set up C-Function callbacks
 */
Event::Event(EventBase& eventBase, int fd, EventType type, EventHandler& eventHandler)
    : event{event_new(eventBase.eventBase, fd, static_cast<short>(type), &eventCallback, &eventHandler)}
{}

Event::Event(EventBase& eventBase, TimerData& timerData)
    : event{evtimer_new(eventBase.eventBase, controlTimerCallback, &timerData)}
{}

EventHandler::EventHandler(JobQueue& jobQueue, Store& store)
    : jobQueue{jobQueue}
    , store{store}
    , finished{false}
    , stopping{false}
    , internalTimerAction{*this}
{
    using namespace std::chrono_literals;
    internalTimerId = addTimer(10'000, internalTimerAction);
    controlTimerAction();
}

void EventHandler::run(std::function<void()>&& notice)
{
    finished = false;
    stopping = false;
    notice();
    eventBase.run();
}

void EventHandler::stopSoft()
{
    ThorsLogDebug("EventHandler", "stopSoft", "Initiating a soft stop. Connection Count: ", store.getOpenConnections());
    if (store.getOpenConnections() == 0) {
        stopHard();
        return;
    }
    stopping = true;
}

void EventHandler::stopHard()
{
    ThorsLogDebug("EventHandler", "stopHard", "Initiating a hard stop. Connection Count: ", store.getOpenConnections());
    finished = true;
}

void EventHandler::add(TASock::Server&& server, ServerCreator&& serverCreator, Pynt& pynt)
{
    int fd = server.socketId();
    store.requestChange(StateUpdateCreateServer{fd,
                                                std::move(server),
                                                std::move(serverCreator),
                                                Event{eventBase, fd, EventType::Read, *this},
                                                pynt
                                               });
}

void EventHandler::add(TASock::SocketStream&& stream, StreamCreator&& streamCreator, Pynt& pynt)
{
    // If we are stopping then we will not accept any more connections.
    if (stopping) {
        ThorsLogDebug("EventHandler", "add", "Ignoring new connection as we are stopping");
        return;
    }
    int fd = stream.getSocket().socketId();
    store.requestChange(StateUpdateCreateStream{fd,
                                                std::move(stream),
                                                std::move(streamCreator),
                                                Event{eventBase, fd, EventType::Read, *this},
                                                Event{eventBase, fd, EventType::Write, *this},
                                                pynt
                                               });
}

int EventHandler::addTimer(int microseconds, TimerAction& action)
{
    int result = nextTImerId++;
    store.requestChange(StateUpdateCreateTimer{result,
                                               microseconds,
                                               &action,
                                               // Event{eventBase, *this, result}
                                               &eventBase,
                                               this
                                              });
    return result;
}

void EventHandler::addOwnedFD(int fd, int owner, EventType initialWait)
{
    store.requestChange(StateUpdateCreateOwnedFD{fd,
                                                 owner,
                                                 initialWait,
                                                 Event{eventBase, fd, EventType::Read, *this},
                                                 Event{eventBase, fd, EventType::Write, *this},
                                                });
}

void EventHandler::remOwnedFD(int fd)
{
    store.requestChange(StateUpdateRemove{fd});
}

void EventHandler::addSharedFD(int fd)
{
    store.requestChange(StateUpdateCreateSharedFD{fd,
                                                  Event{eventBase, fd, EventType::Read, *this},
                                                  Event{eventBase, fd, EventType::Write, *this}
                                                 });
}

void EventHandler::remSharedFD(int fd)
{
    store.requestChange(StateUpdateRemove{fd});
}

void EventHandler::remTimer(int timerId)
{
    store.requestChange(StateUpdateRemove{timerId});
}

void EventHandler::eventAction(int fd, EventType type)
{
    ThorsLogTrace("EventHandler", "eventAction", "Event callback", fd);
    StoreData& info = store.getStoreData(fd);
    std::visit(ApplyEvent{*this, fd, type},  info);
    /* The std::visit ApplyEvent object to call the appropriate of
     * handleServerEvent/ handleStreamEvent/ handleLinkStreamEvent/ handlePipeStreamEvent
     */
}


void EventHandler::handleServerEvent(ServerData& info, int fd, EventType)
{
    ThorsLogTrace("EventHandler", "handleServerEvent", "Connection established");
    addJob(info.coRoutine, fd);
}

void EventHandler::handleStreamEvent(StreamData& info, int fd, EventType type)
{
    ThorsLogTrace("EventHandler", "handleStreamEvent", "Streaming data");
    if (checkFileDescriptorOK(fd, type)) {
        addJob(info.coRoutine, fd);
    }
}

void EventHandler::handleLinkStreamEvent(OwnedFD& info, int fd, EventType)
{
    ThorsLogTrace("EventHandler", "handleLinkStreamEvent", "Link stream");
    addJob(*(info.linkedStreamCoRoutine), fd);
}

void EventHandler::handlePipeStreamEvent(SharedFD& info, int fd, EventType type)
{
    ThorsLogTrace("EventHandler", "handlePipeStreamEvent", "Pipe Streaming");
    std::deque<CoRoutine*>& nextData = (type == EventType::Read) ? info.readWaiting : info.writeWaiting;
    Event&                  nextEvent= (type == EventType::Read) ? info.readEvent   : info.writeEvent;
    if (nextData.size() != 0)
    {
        CoRoutine* next = nextData.front();
        nextData.pop_front();

        addJob(*next, fd);
        if (nextData.size() != 0) {
            nextEvent.add();
        }
    }
}

void EventHandler::handleTimerEvent(TimerData& info, int timerId, EventType /*type*/)
{
    ThorsLogTrace("EventHandler", "handleTimerEvent", "Timer Activated");
    info.timerAction->handleRequest(timerId);
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
        ThorsLogInfo("ThorsAnvil::Nissa::EventHandler", "checkFileDescriptorOK", "Client closed connection");
        store.requestChange(StateUpdateExternallClosed{fd});
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
            // increment the number of active jobs.
            store.incActive();
            if (work()) {
                task = work.get();
            }
        }
        catch (std::exception const& e)
        {
            ThorsLogWarning("ThorsAnvil::Nissa::EventHandler", "addJob", "jobQueue::job: Ignoring Exception: ",  e.what());
        }
        catch (...)
        {
            ThorsLogWarning("ThorsAnvil::Nissa::EventHandler", "addJob", "jobQueue::job: Ignoring Exception: Unknown");
        }
        switch (task.state)
        {
            case TaskYieldState::Remove:
                // Job is being removed. We can decrement the active job count.
                store.decActive();
                store.requestChange(StateUpdateRemove{task.fd});
                break;
            case TaskYieldState::WaitForMore:
                // Waiting for more means that we have finished the current request.
                // But the connection is still open for additional requests.
                // But we are not in the middle of a job is we will decrement the active count.
                store.decActive();
                store.requestChange(StateUpdateRestoreRead{task.fd, fd});
                break;
            case TaskYieldState::RestoreRead:
                // The job has yielded because it is waiting for data from the client.
                // As we are in the middle of a job we will not decrement the count.
                // Note: When the job is restarted (above) it will increment the active count
                //       another time. So the yield return point will decrement the count
                //       to compensate so we don't overcount.
                //       See NisseServer::createStreamJob() (The CoRoutine)
                store.requestChange(StateUpdateRestoreRead{task.fd, fd});
                break;
            case TaskYieldState::RestoreWrite:
                // The job has yielded because it is waiting to write data to the client.
                // As we are in the middle of a job we will not decrement the count.
                // Note: When the job is restarted (above) it will increment the active count
                //       another time. So the yield return point will decrement the count
                //       to compensate so we don't overcount.
                //       See NisseServer::createStreamJob() (The CoRoutine)
                store.requestChange(StateUpdateRestoreWrite{task.fd, fd});
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
    ThorsMessage(8, "EventHandler", "controlTimerAction", "Checking state of connections");
    if (stopping) {
        ThorsLogDebug("EventHandler", "controlTimerAction", "Checking up on soft stop. Connection Count: ", store.getOpenConnections());
        if (store.getOpenConnections() == 0) {
            finished = true;
        }
    }
    if (finished) {
        ThorsLogDebug("EventHandler", "controlTimerAction", "Event Loop breaking now");
        eventBase.loopBreak();
        return;
    }

    // Update all the state information.
    store.processUpdateRequest();
}
