#ifndef THORSANVIL_NISSA_STREAM_STORE_H
#define THORSANVIL_NISSA_STREAM_STORE_H

#include "NissaConfig.h"
#include "Action.h"
#include "EventHandlerLibEvent.h"
#include <ThorsSocket/SocketStream.h>
#include <map>
#include <variant>
#include <vector>
#include <mutex>

namespace ThorsAnvil::Nissa
{

struct StreamData
{
    using SocketStream = ThorsAnvil::ThorsSocket::SocketStream;
    SocketStream        stream;
    Task                task;
    CoRoutine           coRoutine;
    Event               readEvent;
    Event               writeEvent;
};

using CoRoutineCreator = std::function<CoRoutine(StreamData& state)>;

struct StateUpdateCreate
{
    using SocketStream = ThorsAnvil::ThorsSocket::SocketStream;
    int                 fd;
    SocketStream        stream;
    Task                task;
    CoRoutineCreator    coRoutineCreator;
    Event               readEvent;
    Event               writeEvent;
};

struct StateUpdateRemove
{
    int     fd;
};


using StateUpdate = std::variant<StateUpdateCreate, StateUpdateRemove>;

class StreamStore
{
    std::map<int, StreamData>   streamData;
    std::vector<StateUpdate>    updates;
    std::mutex                  updateMutex;

    public:
        StreamData&     getStreamData(int fd);
        void            processUpdateRequest();

        template<typename T>
        void requestChange(T&& update);
    private:
        struct ApplyUpdate
        {
            StreamStore& store;
            ApplyUpdate(StreamStore& store)
                : store(store)
            {}
            void operator()(StateUpdateCreate& update)   {store(update);}
            void operator()(StateUpdateRemove& update)   {store(update);}
        };
        void operator()(StateUpdateCreate& update);
        void operator()(StateUpdateRemove& update);
};

}

#endif
