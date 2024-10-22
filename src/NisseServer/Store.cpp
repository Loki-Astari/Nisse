#include "Store.h"

using namespace ThorsAnvil::Nisse;

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
    auto [iter, ok] = data.insert_or_assign(update.fd,
                                            ServerData{std::move(update.server),
                                                       std::move(invalid),
                                                       std::move(update.readEvent),
                                                       &update.pynt
                                                      });

    ServerData& data = std::get<ServerData>(iter->second);
    data.coRoutine = update.coRoutineCreator(data);
    data.readEvent.add();
};

void Store::operator()(StateUpdateCreateStream& update)
{
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
};

void Store::operator()(StateUpdateRemove& update)
{
    data.erase(update.fd);
}

void Store::operator()(StateUpdateRestoreRead& update)
{
    struct RestoreRead
    {
        void operator()(ServerData& update) {update.readEvent.add();}
        void operator()(StreamData& update) {update.readEvent.add();}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(RestoreRead{}, find->second);
    }
}

void Store::operator()(StateUpdateRestoreWrite& update)
{
    struct RestoreWrite
    {
        void operator()(ServerData&)        {}
        void operator()(StreamData& update) {update.writeEvent.add();}
    };
    auto find = data.find(update.fd);
    if (find != data.end()) {
        std::visit(RestoreWrite{}, find->second);
    }
}
