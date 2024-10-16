#include "NisseServer.h"
#include "EventHandler.h"

namespace TAS   = ThorsAnvil::ThorsSocket;
using namespace ThorsAnvil::Nisse;

NisseServer::NisseServer(int workerCount)
    : jobQueue{workerCount}
    , store{}
    , eventHandler{jobQueue, store}
{}

void NisseServer::run()
{
    eventHandler.run();
}

void NisseServer::stop()
{
    eventHandler.stop();
}

CoRoutine NisseServer::createStreamJob(StreamData& info)
{
    // Exceptions here will be caught
    // By the `EventHandler::addJob()` function.
    return CoRoutine
    {
        [&info](Yield& yield)
        {
            // Set the socket to work asynchronously.
            info.stream.getSocket().setReadYield([&yield](){yield(TaskYieldState::RestoreRead);return true;});
            info.stream.getSocket().setWriteYield([&yield](){yield(TaskYieldState::RestoreWrite);return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield(TaskYieldState::RestoreRead);

            PyntResult result = info.pynt->handleRequest(info.stream);
            while (result == PyntResult::More)
            {
                yield(TaskYieldState::RestoreRead);
                result = info.pynt->handleRequest(info.stream);
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield(TaskYieldState::Remove);
        }
    };
}

CoRoutine NisseServer::createAcceptJob(ServerData& info)
{
    // Exceptions here will be caught
    // By the `EventHandler::addJob()` function.
    return CoRoutine
    {
        [&](Yield& yield)
        {
            // Set the socket to work asynchronously.
            info.server.setYield([&yield](){yield(TaskYieldState::RestoreRead);return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield(TaskYieldState::RestoreRead);

            while (true)
            {
                TAS::Socket     accept = info.server.accept(TAS::Blocking::No);
                if (accept.isConnected())
                {
                    // If everything worked then create a stream connection (see above)
                    // Passing the "Pynt" as the object that will handle the request.
                    // Note: The "Pynt" functionality is not run yet. The socket must be available to use.
                    eventHandler.add(TAS::SocketStream{std::move(accept)}, [&](StreamData& info){return createStreamJob(info);}, *info.pynt);
                }
                yield(TaskYieldState::RestoreRead);
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield(TaskYieldState::Remove);
        }
    };
}

template<typename T>
void NisseServer::listen(T listenerInit, Pynt& pynt)
{
    TAS::Server  server{listenerInit, TAS::Blocking::No};

    eventHandler.add(std::move(server), [&](ServerData& info){return createAcceptJob(info);}, pynt);
}

template void NisseServer::listen<TAS::SServerInfo>(TAS::SServerInfo listenerInit, Pynt& pynt);
template void NisseServer::listen<TAS::ServerInfo>(TAS::ServerInfo listenerInit, Pynt& pynt);
