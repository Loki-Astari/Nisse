#include "Store.h"

using namespace ThorsAnvil::Nissa;

StoreData& Store::getStoreData(int fd)
{
    auto find = streamData.find(fd);
    if (find == streamData.end())
    {
        throw std::runtime_error("BAD");
    }
    return find->second;
}

template<typename T>
void Store::requestChange(T&& update)
{
    std::unique_lock        lock(updateMutex);
    updates.emplace_back(std::move(update));
}

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
    }
    updates.clear();
}

void Store::operator()(StateUpdateCreateStream& update)
{
    static CoRoutine    invalid{[](Yield&){}};

    auto [iter, ok] = streamData.insert_or_assign(update.fd,
                                                  StreamData{std::move(update.stream),
                                                             std::move(update.task),
                                                             std::move(invalid),
                                                             std::move(update.readEvent),
                                                             std::move(update.writeEvent)
                                                            });

    StreamData& data = std::get<StreamData>(iter->second);
    data.coRoutine = update.coRoutineCreator(data);
    data.readEvent.add();
};

void Store::operator()(StateUpdateRemove& update)
{
    streamData.erase(update.fd);
}

void Store::operator()(StateUpdateRestoreRead& update)
{
    struct RestoreRead
    {
        void operator()(StreamData& update) {update.readEvent.add();}
    };
    auto find = streamData.find(update.fd);
    if (find != streamData.end()) {
        std::visit(RestoreRead{}, find->second);
    }
}

void Store::operator()(StateUpdateRestoreWrite& update)
{
    struct RestoreRead
    {
        void operator()(StreamData& update) {update.writeEvent.add();}
    };
    auto find = streamData.find(update.fd);
    if (find != streamData.end()) {
        std::visit(RestoreRead{}, find->second);
    }
}
