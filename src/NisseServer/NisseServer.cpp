#include "NisseServer.h"
#include "EventHandler.h"
#include "Context.h"

namespace TASock   = ThorsAnvil::ThorsSocket;

using namespace ThorsAnvil::Nisse::Server;

NisseServer::NisseServer(std::size_t workerCount)
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
            TASock::Socket& streamSocket = info.stream.getSocket();

            streamSocket.setReadYield([&yield, &info, socketId]()
            {
                // If yield() throws we are unwinding the stack.
                // This lambda is being called from deep inside the iostream but we want the
                // exception to propagate out of the stream, thus we set the exception bit here,
                // but if yield() does not throw put the exception mask back.
                std::ios_base::iostate e = info.stream.exceptions();
                info.stream.exceptions(std::ios::badbit);
                yield({TaskYieldState::RestoreRead, socketId});
                info.stream.exceptions(e);
                return true;
            });
            streamSocket.setWriteYield([&yield, &info, socketId]()
            {
                // If yield() throws we are unwinding the stack.
                // This lambda is being called from deep inside the iostream but we want the
                // exception to propagate out of the stream, thus we set the exception bit here,
                // but if yield() does not throw put the exception mask back.
                std::ios_base::iostate e = info.stream.exceptions();
                info.stream.exceptions(std::ios::badbit);
                yield({TaskYieldState::RestoreWrite, socketId});
                info.stream.exceptions(e);
                return true;
            });

            // Return control to the creator.
            // The next call will happen when there is data available on the file descriptor.
            // We do this as the co-routine is created outside a JobQueue context.
            // once we return a Job will be added to correctly continue the Job.
            // See Store: StateUpdateCreateStream and StateUpdateCreateServer
            yield({TaskYieldState::RestoreRead, socketId});

            // On normal sockets this does nothing.
            // ON SSL we do the SSL handshake.
            streamSocket.deferredAccept();

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
                TASock::Socket     accept = info.server.accept(TASock::Blocking::No, TASock::DeferAccept::Yes);
                if (accept.isConnected())
                {
                    // If everything worked then create a stream connection (see above)
                    // Passing the "Pynt" as the object that will handle the request.
                    // Note: The "Pynt" functionality is not run yet. The socket must be available to use.
                    eventHandler.add(TASock::SocketStream{std::move(accept)}, [&](StreamData& info){return createStreamJob(info);}, *info.pynt);
                }
                yield({TaskYieldState::RestoreRead, socketId});
            }
            // We are all done
            // So indicate that we should tidy up state.
            yield({TaskYieldState::Remove, socketId});
        }
    };
}

void NisseServer::listen(TASock::ServerInit&& listenerInit, Pynt& pynt)
{
    TASock::Server  server{std::move(listenerInit), TASock::Blocking::No};

    eventHandler.add(std::move(server), [&](ServerData& info){return createAcceptJob(info);}, pynt);
}

void NisseServer::addResourceQueue(int fd)
{
    eventHandler.addResourceQueue(fd);
}

void NisseServer::remResourceQueue(int fd)
{
    eventHandler.remResourceQueue(fd);
}
