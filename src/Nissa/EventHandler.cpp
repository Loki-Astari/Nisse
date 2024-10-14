#include "EventHandler.h"
#include "JobQueue.h"
#include "StreamStore.h"

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

EventHandler::EventHandler(JobQueue& jobQueue, StreamStore& streamStore)
    : jobQueue(jobQueue)
    , streamStore(streamStore)
    , timer(eventBase, *this)
{
    timer.add(controlTimerPause);
}

void EventHandler::run()
{
    eventBase.run();
}

void EventHandler::add(int fd, ThorsAnvil::ThorsSocket::SocketStream&& stream, Task&& task)
{
    streamStore.requestChange(StateUpdateCreate{fd,
                                                std::move(stream),
                                                std::move(task),
                                                [&](StreamData& info){return buildCoRoutine(info);},
                                                Event{eventBase, fd, EV_READ, *this},
                                                Event{eventBase, fd, EV_WRITE, *this},
                                               });
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
    return !(result == 0 || (result == -1 && errno != EAGAIN && errno != EWOULDBLOCK));
}

void EventHandler::eventHandle(int fd, EventType type)
{
    StreamData& info = streamStore.getStreamData(fd);

    /*
     * If the socket was closed on the other end.
     * Then remove it and all its data.
     */
    if (info.stream.getSocket().isConnected() && !checkFileDescriptorOK(fd, type))
    {
        std::cout << "Remove Socket\n";
        streamStore.requestChange(StateUpdateRemove{fd});
        return;
    }
    /*
     * Add a lamda to the JobQueue to read/write data from the stream
     * using the stored "Task via the CoRoutine.
     */
    jobQueue.addJob([&]()
    {
        TaskYieldState task = TaskYieldState::Remove;
        if (info.coRoutine()) {
            task = info.coRoutine.get();
        }
        switch (task)
        {
            case TaskYieldState::Remove:
                streamStore.requestChange(StateUpdateRemove{fd});
                break;
            case TaskYieldState::RestoreRead:
                streamStore.requestChange(StateUpdateRestoreRead{fd});
                break;
            case TaskYieldState::RestoreWrite:
                streamStore.requestChange(StateUpdateRestoreWrite{fd});
                break;
        }
    });
}

CoRoutine EventHandler::buildCoRoutine(StreamData& info)
{
    return CoRoutine
    {
        [&info](Yield& yield)
        {
            info.stream.getSocket().setReadYield([&yield](){yield(TaskYieldState::RestoreRead);return true;});
            info.stream.getSocket().setWriteYield([&yield](){yield(TaskYieldState::RestoreWrite);return true;});
            yield(TaskYieldState::RestoreRead);
            info.task(info.stream, yield);
            yield(TaskYieldState::Remove);
        }
    };
}

/*
 * Callback function controlled by timer.
 * This is used to processes all pending changes.
 *
 * Thus all changes to state are done by the main thread.
 */
void EventHandler::controlTimerAction()
{
    streamStore.processUpdateRequest();
    // Put the timer back.
    timer.add(controlTimerPause);
}
