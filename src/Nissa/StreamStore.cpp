#include "StreamStore.h"

using namespace ThorsAnvil::Nissa;

StreamData& StreamStore::getStreamData(int fd)
{
    auto find = streamData.find(fd);
    if (find == streamData.end())
    {
        throw std::runtime_error("BAD");
    }
    return find->second;
}

template<typename T>
void StreamStore::requestChange(T&& update)
{
    std::unique_lock        lock(updateMutex);
    updates.emplace_back(std::move(update));
}

template void StreamStore::requestChange<StateUpdateCreate>(StateUpdateCreate&& update);
template void StreamStore::requestChange<StateUpdateRemove>(StateUpdateRemove&& update);

void StreamStore::processUpdateRequest()
{
    std::unique_lock        lock(updateMutex);
    ApplyUpdate             updater{*this};
    for (auto& update: updates)
    {
        std::visit(updater, update);
    }
    updates.clear();
}

void StreamStore::operator()(StateUpdateCreate& update)
{
    static CoRoutine    invalid{[](Yield&){}};

    auto [iter, ok] = streamData.insert_or_assign(update.fd,
                                                  StreamData{std::move(update.stream),
                                                             std::move(update.task),
                                                             std::move(invalid),
                                                             std::move(update.readEvent),
                                                             std::move(update.writeEvent)
                                                            });

    iter->second.coRoutine = update.coRoutineCreator(iter->second);
    iter->second.readEvent.add();
};

void StreamStore::operator()(StateUpdateRemove& update)
{
    streamData.erase(update.fd);
}

void StreamStore::operator()(StateUpdateRestoreRead& update)
{
    auto find = streamData.find(update.fd);
    if (find != streamData.end()) {
        find->second.readEvent.add();
    }
}

void StreamStore::operator()(StateUpdateRestoreWrite& update)
{
    auto find = streamData.find(update.fd);
    if (find != streamData.end()) {
        find->second.writeEvent.add();
    }
}
