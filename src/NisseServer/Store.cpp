#include "Store.h"
#include "EventHandlerLibEvent.h"

using namespace ThorsAnvil::Nisse::Server;

/*
 * There is no default constructor for CoRoutine.
 * But a non-coroutine object is allowed to exist.
 * So we have a single invalid co-routine that can be used
 * to initialize non-coroutine objects until we are ready to initialize them.
 */
CoRoutine Store::invalid{[](Yield&){}};

StoreData& Store::getStoreData(int fd)
{
    auto find = data.find(fd);
    if (find == data.end())
    {
        throw std::runtime_error("Invalid Request: Exit applications");
    }
    return find->second;
}

template<typename T>
void Store::requestChange(T&& update)
{
    std::unique_lock        lock(updateMutex);
    updates.emplace_back(std::move(update));
}

template void Store::requestChange<StateUpdateCreateServer>(StateUpdateCreateServer&& update);
template void Store::requestChange<StateUpdateCreateStream>(StateUpdateCreateStream&& update);
template void Store::requestChange<StateUpdateCreateOwnedFD>(StateUpdateCreateOwnedFD&& update);
template void Store::requestChange<StateUpdateCreateSharedFD>(StateUpdateCreateSharedFD&& update);
template void Store::requestChange<StateUpdateCreateTimer>(StateUpdateCreateTimer&& update);
template void Store::requestChange<StateUpdateExternallClosed>(StateUpdateExternallClosed&& update);
template void Store::requestChange<StateUpdateRemove>(StateUpdateRemove&& update);
template void Store::requestChange<StateUpdateRestoreRead>(StateUpdateRestoreRead&& update);
template void Store::requestChange<StateUpdateRestoreWrite>(StateUpdateRestoreWrite&& update);

void Store::processUpdateRequest()
{
    std::unique_lock        lock(updateMutex);
    ApplyUpdate             updater{*this};
    for (auto& update: updates)
    {
        std::visit(updater, update);
        /* This visit calls one of the operators below based on the type of the request */
    }
    updates.clear();
}

void Store::operator()(StateUpdateCreateServer& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateServer&)", "Start: ", update.fd);
    auto [iter, ok] = data.insert_or_assign(update.fd,
                                            ServerData{std::move(update.server),
                                                       std::move(invalid),
                                                       std::move(update.readEvent),
                                                       &update.pynt
                                                      });

    ServerData& data = std::get<ServerData>(iter->second);
    data.coRoutine = update.coRoutineCreator(data);
    data.readEvent.add();
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateServer&)", "DONE");
}

void Store::operator()(StateUpdateCreateStream& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateStream&)", "Start: ", update.fd);
    auto [iter, ok] = data.insert_or_assign(update.fd,
                                            StreamData{std::move(update.stream),
                                                       std::move(invalid),
                                                       std::move(update.readEvent),
                                                       std::move(update.writeEvent),
                                                       &update.pynt
                                                      });

    StreamData& data = std::get<StreamData>(iter->second);
    data.coRoutine = update.coRoutineCreator(data);
    data.readEvent.add();
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateStream&)", "DONE");
}

void Store::operator()(StateUpdateCreateOwnedFD& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateOwnedFD&)", "Start: ", update.fd);
    auto find = data.find(update.linkedStream);
    if (find == data.end()) {
        return;
    }
    StoreData&  linkedData          = find->second;
    StreamData& linkedStreamData    = std::get<StreamData>(linkedData);
    CoRoutine&  linkedStreamCo      = linkedStreamData.coRoutine;

    auto [iter, ok] = data.insert_or_assign(update.fd,
                                            OwnedFD{&linkedStreamCo,
                                                    std::move(update.readEvent),
                                                    std::move(update.writeEvent)
                                                   });
    OwnedFD& data = std::get<OwnedFD>(iter->second);
    if (update.initialWait == EventType::Read) {
        data.readEvent.add();
    }
    else {
        data.writeEvent.add();
    }
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateOwnedFD&)", "DONE");
}

void Store::operator()(StateUpdateCreateSharedFD& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateSharedFD&)", "Start: ", update.fd);
    auto [iter, ok] = data.insert_or_assign(update.fd,
                                            SharedFD{{},    // Empty Read List
                                                     {},    // Empty Write List
                                                     std::move(update.readEvent),
                                                     std::move(update.writeEvent),
                                                    });
    ((void)iter);
    ((void)ok);
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateSharedFD&)", "DONE");
}

void Store::operator()(StateUpdateCreateTimer& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateTimer&)", "Start: ", update.timerId);
    auto [iter, ok] = data.insert_or_assign(update.timerId,
                                            TimerData{update.timerId,
                                                      update.waitTime,
                                                      update.timerAction,
                                                      update.eventHandler,
                                                      {}
                                                    });
    TimerData& data = std::get<TimerData>(iter->second);
    data.timerEvent = Event{*update.eventBase, data};
    data.timerEvent.add(update.waitTime);
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateTimer&)", "DONE");
}

void Store::operator()(StateUpdateExternallClosed& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateExternallClosed&)", "Start: ", update.fd);
    struct ExternallyClosed
    {
        Store&  store;

        void operator()(ServerData&)            {}
        void operator()(StreamData& data)       {data.stream.getSocket().externalyClosed();store.decActive();}
        void operator()(OwnedFD&)               {}
        void operator()(SharedFD&)              {}
        void operator()(TimerData&)             {}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(ExternallyClosed{*this}, find->second);
    }
    data.erase(update.fd);
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateExternallClosed&)", "DONE");
}

void Store::operator()(StateUpdateRemove& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRemove&)", "Start: ", update.fd);
    auto find = data.find(update.fd);
    if (find != std::end(data)) {
        if (std::holds_alternative<StreamData>(find->second)) {
            decActive();
        }
    }
    data.erase(update.fd);
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRemove&)", "DONE");
}

void Store::operator()(StateUpdateRestoreRead& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreRead&)", "Start: ", update.fd);
    struct RestoreRead
    {
        Store&                      store;
        StateUpdateRestoreRead&     update;
        RestoreRead(Store& store, StateUpdateRestoreRead& update)
            : store(store)
            , update(update)
        {}
        void operator()(ServerData& data)       {data.readEvent.add();}
        void operator()(StreamData& data)       {data.readEvent.add();store.decActive();}
        void operator()(OwnedFD& data)          {data.readEvent.add();}
        void operator()(SharedFD& data)
        {
            auto find = store.data.find(update.owner);
            if (find == store.data.end()) {
                return;
            }
            StoreData&  ownerDataRef    = find->second;
            if (std::holds_alternative<StreamData>(ownerDataRef))
            {
                StreamData& ownerStreamRef  = std::get<StreamData>(ownerDataRef);
                data.readWaiting.emplace_back(&ownerStreamRef.coRoutine);
                data.readEvent.add();
            }
        }
        void operator()(TimerData&)             {}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(RestoreRead{*this, update}, find->second);
    }
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreRead&)", "DONE");
}

void Store::operator()(StateUpdateRestoreWrite& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreWrite&)", "Start: ", update.fd);
    struct RestoreWrite
    {
        Store&                      store;
        StateUpdateRestoreWrite&    update;
        RestoreWrite(Store& store, StateUpdateRestoreWrite& update)
            : store(store)
            , update(update)
        {}
        void operator()(ServerData&)            {}
        void operator()(StreamData& data)       {data.writeEvent.add();}
        void operator()(OwnedFD& data)          {data.writeEvent.add();}
        void operator()(SharedFD& data)
        {
            auto find = store.data.find(update.owner);
            if (find == store.data.end()) {
                return;
            }
            StoreData&  ownerDataRef    = find->second;
            if (std::holds_alternative<StreamData>(ownerDataRef))
            {
                StreamData& ownerStreamRef  = std::get<StreamData>(ownerDataRef);
                data.writeWaiting.emplace_back(&ownerStreamRef.coRoutine);
                data.writeEvent.add();
            }
        }
        void operator()(TimerData&)             {}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(RestoreWrite{*this, update}, find->second);
    }
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreWrite&)", "DONE");
}
