#ifndef THORSANVIL_NISSA_STORE_H
#define THORSANVIL_NISSA_STORE_H

#include "NissaConfig.h"
#include "Action.h"
#include "EventHandlerLibEvent.h"
#include <ThorsSocket/Server.h>
#include <ThorsSocket/SocketStream.h>
#include <map>
#include <variant>
#include <vector>
#include <mutex>

namespace ThorsAnvil::Nissa
{

struct ServerData
{
    using Server = ThorsAnvil::ThorsSocket::Server;
    Server              server;
    Task                task;
    CoRoutine           coRoutine;
    Event               readEvent;
};
struct StreamData
{
    using SocketStream = ThorsAnvil::ThorsSocket::SocketStream;
    SocketStream        stream;
    Task                task;
    CoRoutine           coRoutine;
    Event               readEvent;
    Event               writeEvent;
};

using StoreData = std::variant<ServerData, StreamData>;

using CoRoutineServerCreator = std::function<CoRoutine(ServerData& state)>;
using CoRoutineStreamCreator = std::function<CoRoutine(StreamData& state)>;

struct StateUpdateCreateServer
{
    using Server = ThorsAnvil::ThorsSocket::Server;
    int                     fd;
    Server                  server;
    Task                    task;
    CoRoutineServerCreator  coRoutineCreator;
    Event                   readEvent;
};

struct StateUpdateCreateStream
{
    using SocketStream = ThorsAnvil::ThorsSocket::SocketStream;
    int                     fd;
    SocketStream            stream;
    Task                    task;
    CoRoutineStreamCreator  coRoutineCreator;
    Event                   readEvent;
    Event                   writeEvent;
};

struct StateUpdateRemove
{
    int     fd;
};

struct StateUpdateRestoreRead
{
    int fd;
};

struct StateUpdateRestoreWrite
{
    int fd;
};


using StateUpdate = std::variant<StateUpdateCreateServer, StateUpdateCreateStream, StateUpdateRemove, StateUpdateRestoreRead, StateUpdateRestoreWrite>;

class Store
{
    std::map<int, StoreData>   streamData;
    std::vector<StateUpdate>    updates;
    std::mutex                  updateMutex;

    public:
        StoreData&      getStoreData(int fd);
        void            processUpdateRequest();

        template<typename T>
        void requestChange(T&& update);
    private:
        struct ApplyUpdate
        {
            Store& store;
            ApplyUpdate(Store& store)
                : store(store)
            {}
            void operator()(StateUpdateCreateServer& update){store(update);}
            void operator()(StateUpdateCreateStream& update){store(update);}
            void operator()(StateUpdateRemove& update)      {store(update);}
            void operator()(StateUpdateRestoreRead& update) {store(update);}
            void operator()(StateUpdateRestoreWrite& update){store(update);}
        };
        void operator()(StateUpdateCreateServer& update);
        void operator()(StateUpdateCreateStream& update);
        void operator()(StateUpdateRemove& update);
        void operator()(StateUpdateRestoreRead& update);
        void operator()(StateUpdateRestoreWrite& update);
};

}

#endif
