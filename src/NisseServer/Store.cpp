#include "Store.h"

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
template void Store::requestChange<StateUpdateCreateLinkStream>(StateUpdateCreateLinkStream&& update);
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

void Store::operator()(StateUpdateCreateLinkStream& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateLinkStream&)", "Start: ", update.fd);
    auto find = data.find(update.linkedStream);
    if (find == data.end()) {
        return;
    }
    StoreData&  linkedData          = find->second;
    StreamData& linkedStreamData    = std::get<StreamData>(linkedData);
    CoRoutine&  linkedStreamCo      = linkedStreamData.coRoutine;

    auto [iter, ok] = data.insert_or_assign(update.fd,
                                            LinkedStreamData{&linkedStreamCo,
                                                             std::move(update.readEvent),
                                                             std::move(update.writeEvent)
                                                            });
    LinkedStreamData& data = std::get<LinkedStreamData>(iter->second);
    if (update.initialWait == EventType::Read) {
        data.readEvent.add();
    }
    else {
        data.writeEvent.add();
    }
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateCreateLinkStream&)", "DONE");
}

void Store::operator()(StateUpdateExternallClosed& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateExternallClosed&)", "Start: ", update.fd);
    struct ExternallyClosed
    {
        void operator()(ServerData&)        {}
        void operator()(StreamData& update) {update.stream.getSocket().externalyClosed();}
        void operator()(LinkedStreamData&)  {}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(ExternallyClosed{}, find->second);
    }
    data.erase(update.fd);
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateExternallClosed&)", "DONE");
}

void Store::operator()(StateUpdateRemove& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRemove&)", "Start: ", update.fd);
    data.erase(update.fd);
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRemove&)", "DONE");
}

void Store::operator()(StateUpdateRestoreRead& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreRead&)", "Start: ", update.fd);
    struct RestoreRead
    {
        void operator()(ServerData& update)         {update.readEvent.add();}
        void operator()(StreamData& update)         {update.readEvent.add();}
        void operator()(LinkedStreamData& update)   {update.readEvent.add();}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(RestoreRead{}, find->second);
    }
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreRead&)", "DONE");
}

void Store::operator()(StateUpdateRestoreWrite& update)
{
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreWrite&)", "Start: ", update.fd);
    struct RestoreWrite
    {
        void operator()(ServerData&)                {}
        void operator()(StreamData& update)         {update.writeEvent.add();}
        void operator()(LinkedStreamData& update)   {update.writeEvent.add();}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(RestoreWrite{}, find->second);
    }
    ThorsLogDebug("ThorsAnvil::NisseServer::Store", "operator()(StateUpdateRestoreWrite&)", "DONE");
}
