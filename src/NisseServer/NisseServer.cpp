#include "NisseServer.h"
#include "EventHandler.h"
#include "Context.h"

namespace TAS   = ThorsAnvil::ThorsSocket;
using namespace ThorsAnvil::Nisse::Server;

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
        [&info, &server = *this](Yield& yield)
        {
            int socketId = info.stream.getSocket().socketId();
            Context     context{server, yield, socketId};
            // Set the socket to work asynchronously.
            info.stream.getSocket().setReadYield([&yield, socketId](){yield({TaskYieldState::RestoreRead, socketId});return true;});
            info.stream.getSocket().setWriteYield([&yield, socketId](){yield({TaskYieldState::RestoreWrite, socketId});return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield({TaskYieldState::RestoreRead, socketId});

            PyntResult result = info.pynt->handleRequest(info.stream, context);
            while (result == PyntResult::More)
            {
                yield({TaskYieldState::RestoreRead, socketId});
                result = info.pynt->handleRequest(info.stream, context);
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield({TaskYieldState::Remove, socketId});
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
            int socketId = info.server.socketId();
            // Set the socket to work asynchronously.
            info.server.setYield([&yield, socketId](){yield({TaskYieldState::RestoreRead, socketId});return true;});

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            yield({TaskYieldState::RestoreRead, socketId});

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
                yield({TaskYieldState::RestoreRead, socketId});
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield({TaskYieldState::Remove, socketId});
        }
    };
}

void NisseServer::listen(TAS::ServerInit&& listenerInit, Pynt& pynt)
{
    TAS::Server  server{std::move(listenerInit), TAS::Blocking::No};

    eventHandler.add(std::move(server), [&](ServerData& info){return createAcceptJob(info);}, pynt);
}
